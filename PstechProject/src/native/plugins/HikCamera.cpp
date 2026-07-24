#include "HikCamera.h"
#include <iostream>
#include <cstdio>
#include <thread>
#include <atomic>

ICamera* CreateHikCamera() { return new HikCamera(); }

// PlayM4 解码回调在 Win64 下第三参(nUser)为 32 位 long，无法承载 64 位 this 指针，
// 故用文件内静态实例指针在回调中访问当前对象（HikCamera 为单例使用）。
static std::atomic<HikCamera*> g_hikDecInst{nullptr};

HikCamera::HikCamera()
    : m_lUserID(-1), m_lRealPlayHandle(-1), m_nPort(-1), m_stopping(false), m_sdkInitialized(false) {}
HikCamera::~HikCamera() { Stop(); }

void HikCamera::SetRecoveryConfig(bool enabled, int frameTimeoutMs, int sdkReconnectIntervalMs, int hardRecoveryTimeoutMs) {
    ICamera::SetRecoveryConfig(enabled, frameTimeoutMs, sdkReconnectIntervalMs, hardRecoveryTimeoutMs);
    if (m_sdkInitialized) ApplySdkReconnectConfig();
}

void HikCamera::ApplySdkReconnectConfig() {
    const BOOL enabled = m_reconnectEnabled.load() ? TRUE : FALSE;
    if (!NET_DVR_SetReconnect(static_cast<DWORD>(m_sdkReconnectIntervalMs.load()), enabled)) {
        std::cerr << "[Hik] NET_DVR_SetReconnect failed, error=" << NET_DVR_GetLastError() << std::endl;
    }
}

bool HikCamera::InitInternal(const std::string& ip, const std::string& user, const std::string& pass, int port) {
    std::cout << "--- [Hik] InitInternal ---" << std::endl;
    std::cout << "IP: [" << ip << "] User: [" << user << "]" << std::endl;
    std::cout << "Port: " << port << std::endl;

    m_ip = ip;
    NotifyStatus(STATUS_CONNECTING, "HikSDK Init...");
    if (!NET_DVR_Init()) {
        NotifyStatus(STATUS_ERROR_DEVICE, "Hik SDK initialization failed");
        return false;
    }
    m_sdkInitialized = true;
    m_stopping.store(false);

    if (!NET_DVR_SetExceptionCallBack_V30(0, nullptr, ExceptionCallback, this)) {
        std::cerr << "[Hik] NET_DVR_SetExceptionCallBack_V30 failed, error=" << NET_DVR_GetLastError() << std::endl;
    }
    NET_DVR_SetConnectTime(2000, 1);
    ApplySdkReconnectConfig();
    
    NET_DVR_USER_LOGIN_INFO loginInfo = {0};
    NET_DVR_DEVICEINFO_V40 deviceInfo = {0};
    std::snprintf(loginInfo.sDeviceAddress, sizeof(loginInfo.sDeviceAddress), "%s", ip.c_str());
    loginInfo.wPort = port;
    std::snprintf(loginInfo.sUserName, sizeof(loginInfo.sUserName), "%s", user.c_str());
    std::snprintf(loginInfo.sPassword, sizeof(loginInfo.sPassword), "%s", pass.c_str());
    
    m_lUserID = NET_DVR_Login_V40(&loginInfo, &deviceInfo);
    if (m_lUserID < 0) {
        // 打印详细错误码
        int err = NET_DVR_GetLastError();
        std::cerr << "[Hik] Login Failed. ErrCode: " << err << std::endl;
        NotifyStatus(STATUS_ERROR_AUTH, "Login Failed");
        return false;
    }
    NotifyStatus(STATUS_CONNECTING, "Hik login succeeded; waiting for preview");
    return true;
}

bool HikCamera::Start() {
    if (m_lUserID < 0) return false;
    m_stopping.store(false);
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.lChannel = 1; 
    struPlayInfo.dwStreamType = 0; 
    struPlayInfo.dwLinkMode = 0;
    struPlayInfo.bBlocked = 1; 
    m_lRealPlayHandle = NET_DVR_RealPlay_V40(m_lUserID, &struPlayInfo, RealDataCallBack, this);
    if (m_lRealPlayHandle < 0) {
        std::cerr << "[Hik] NET_DVR_RealPlay_V40 failed, error=" << NET_DVR_GetLastError() << std::endl;
        NotifyStatus(STATUS_ERROR_DEVICE, "Hik preview start failed");
        return false;
    }
    NotifyStatus(STATUS_WAITING_FRAME, "Hik preview started; waiting for first frame");
    return true;
}

void HikCamera::Stop() {
    if (m_stopping.exchange(true)) return;
    m_streamAvailable.store(false);
    if (m_lRealPlayHandle >= 0) { NET_DVR_StopRealPlay(m_lRealPlayHandle); m_lRealPlayHandle = -1; }
    CloseDecoder();
    if (m_lUserID >= 0) { NET_DVR_Logout(m_lUserID); m_lUserID = -1; }
    HikCamera* expected = this;
    g_hikDecInst.compare_exchange_strong(expected, nullptr);
    if (m_sdkInitialized) {
        NET_DVR_Cleanup();
        m_sdkInitialized = false;
    }
    NotifyStatus(STATUS_DISCONNECTED, "Hik camera stopped");
}

void CALLBACK HikCamera::ExceptionCallback(DWORD dwType, LONG lUserID, LONG lHandle, void* pUser) {
    HikCamera* camera = static_cast<HikCamera*>(pUser);
    if (!camera || camera->m_stopping.load()) return;
    if (camera->m_lUserID >= 0 && lUserID >= 0 && lUserID != camera->m_lUserID) return;

    switch (dwType) {
        case EXCEPTION_PREVIEW:
        case EXCEPTION_RECONNECT:
        case EXCEPTION_RELOGIN:
            camera->MarkStreamUnavailable(STATUS_RECONNECTING, "Hik SDK is reconnecting");
            break;
        case PREVIEW_RECONNECTSUCCESS:
        case RELOGIN_SUCCESS:
            camera->MarkTransportRecovered("Hik connection recovered; waiting for video frame");
            break;
        case EXCEPTION_RELOGIN_FAILED:
            camera->MarkStreamUnavailable(STATUS_ERROR_NETWORK, "Hik SDK reconnect failed");
            break;
        default:
            break;
    }
}

void HikCamera::CloseDecoder() {
    std::lock_guard<std::mutex> lock(m_decoderMutex);
    if (m_nPort >= 0) {
        PlayM4_Stop(m_nPort);
        PlayM4_CloseStream(m_nPort);
        PlayM4_FreePort(m_nPort);
        m_nPort = -1;
    }
}

bool HikCamera::ResetDecoder(const BYTE* systemHeader, DWORD headerSize) {
    std::lock_guard<std::mutex> lock(m_decoderMutex);
    if (m_nPort >= 0) {
        PlayM4_Stop(m_nPort);
        PlayM4_CloseStream(m_nPort);
        PlayM4_FreePort(m_nPort);
        m_nPort = -1;
    }
    if (!systemHeader || headerSize == 0 || !PlayM4_GetPort(&m_nPort)) return false;

    g_hikDecInst.store(this);
    if (!PlayM4_SetStreamOpenMode(m_nPort, STREAME_REALTIME)
        || !PlayM4_OpenStream(m_nPort, const_cast<BYTE*>(systemHeader), headerSize, 1024 * 1024)) {
        PlayM4_FreePort(m_nPort);
        m_nPort = -1;
        return false;
    }
#ifdef _WIN32
    PlayM4_SetDecCallBackMend(m_nPort, DecCallBack, 0);
#else
    PlayM4_SetDecCallBackMend(m_nPort, DecCallBack, this);
#endif
    if (!PlayM4_Play(m_nPort, 0)) {
        PlayM4_CloseStream(m_nPort);
        PlayM4_FreePort(m_nPort);
        m_nPort = -1;
        return false;
    }
    return true;
}

void CALLBACK HikCamera::RealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser) {
    HikCamera* pThis = (HikCamera*)pUser;
    if (!pThis || pThis->m_stopping.load()) return;
    if (dwDataType == NET_DVR_SYSHEAD) {
        if (!pThis->ResetDecoder(pBuffer, dwBufSize)) {
            pThis->MarkStreamUnavailable(STATUS_ERROR_DEVICE, "Hik decoder initialization failed");
        }
    } else if (dwDataType == NET_DVR_STREAMDATA && dwBufSize > 0) {
        std::lock_guard<std::mutex> lock(pThis->m_decoderMutex);
        if (pThis->m_nPort >= 0) PlayM4_InputData(pThis->m_nPort, pBuffer, dwBufSize);
    }
}

#ifdef _WIN32
void CALLBACK HikCamera::DecCallBack(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nUser, long nReserved2) {
    HikCamera* pThis = g_hikDecInst.load(); // Win64 下 nUser(long) 无法承载 64 位指针，改用静态实例
    if(pThis) pThis->ProcessDecodedFrame(pBuf, (int)nSize, pFrameInfo);
}
#else
void CALLBACK HikCamera::DecCallBack(int nPort, char * pBuf, int nSize, FRAME_INFO * pFrameInfo, void* nUser, int nReserved2) {
    HikCamera* pThis = (HikCamera*)nUser;
    if(pThis) pThis->ProcessDecodedFrame(pBuf, nSize, pFrameInfo);
}
#endif

void HikCamera::ProcessDecodedFrame(char* pBuf, int nSize, FRAME_INFO* pFrameInfo) {
    if (m_stopping.load() || !pBuf || !pFrameInfo) return;
    if (pFrameInfo->nType == T_YV12) {
        cv::Mat yv12(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
        cv::Mat bgr;
        cv::cvtColor(yv12, bgr, cv::COLOR_YUV2BGR_YV12);
        PushFrame(bgr);
    }
}

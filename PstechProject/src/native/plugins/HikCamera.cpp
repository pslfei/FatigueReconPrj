#include "HikCamera.h"
#include <iostream>
#include <cstdio>
#include <thread>

// [诊断函数]
void HexDump(const char* label, const char* str) {
    if (!str) { std::cout << label << ": [NULL]" << std::endl; return; }
    std::cout << label << ": [" << str << "] Hex: ";
    const unsigned char* p = (const unsigned char*)str;
    while (*p) {
        printf("%02X ", *p);
        p++;
    }
    printf("\n");
}

ICamera* CreateHikCamera() { return new HikCamera(); }

// PlayM4 解码回调在 Win64 下第三参(nUser)为 32 位 long，无法承载 64 位 this 指针，
// 故用文件内静态实例指针在回调中访问当前对象（HikCamera 为单例使用）。
static HikCamera* g_hikDecInst = nullptr;

HikCamera::HikCamera() : m_lUserID(-1), m_lRealPlayHandle(-1), m_nPort(-1), m_mockRunning(false) {}
HikCamera::~HikCamera() { Stop(); }

bool HikCamera::InitInternal(const std::string& ip, const std::string& user, const std::string& pass, int port) {
    // [诊断] 打印参数
    std::cout << "--- [Hik] InitInternal ---" << std::endl;
    HexDump("IP", ip.c_str());
    HexDump("User", user.c_str());
    HexDump("Pass", pass.c_str());
    std::cout << "Port: " << port << std::endl;

    m_ip = ip;
    NotifyStatus(STATUS_CONNECTING, "HikSDK Init...");
    NET_DVR_Init();
    NET_DVR_SetConnectTime(2000, 1);
    
    NET_DVR_USER_LOGIN_INFO loginInfo = {0};
    NET_DVR_DEVICEINFO_V40 deviceInfo = {0};
    strcpy(loginInfo.sDeviceAddress, ip.c_str());
    loginInfo.wPort = port;
    strcpy(loginInfo.sUserName, user.c_str());
    strcpy(loginInfo.sPassword, pass.c_str());
    
    m_lUserID = NET_DVR_Login_V40(&loginInfo, &deviceInfo);
    if (m_lUserID < 0) {
        // 打印详细错误码
        int err = NET_DVR_GetLastError();
        std::cerr << "[Hik] Login Failed. ErrCode: " << err << ". Soft fail -> Mock Mode." << std::endl;
        NotifyStatus(STATUS_ERROR_AUTH, "Login Failed");
        return true; 
    }
    NotifyStatus(STATUS_CONNECTED, "Hik Login Success");
    return true;
}

bool HikCamera::Start() {
    if (m_lUserID < 0) {
        if (m_mockRunning) return true;
        m_mockRunning = true;
        m_mockThread = std::thread(&HikCamera::MockLoop, this);
        return true; 
    }
    // ... (后续保持不变) ...
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.lChannel = 1; 
    struPlayInfo.dwStreamType = 0; 
    struPlayInfo.dwLinkMode = 0;
    struPlayInfo.bBlocked = 1; 
    m_lRealPlayHandle = NET_DVR_RealPlay_V40(m_lUserID, &struPlayInfo, RealDataCallBack, this);
    return (m_lRealPlayHandle >= 0);
}

void HikCamera::Stop() {
    m_mockRunning = false;
    if (m_mockThread.joinable()) m_mockThread.join();
    if (m_lRealPlayHandle >= 0) { NET_DVR_StopRealPlay(m_lRealPlayHandle); m_lRealPlayHandle = -1; }
    if (m_nPort >= 0) { PlayM4_Stop(m_nPort); PlayM4_CloseStream(m_nPort); PlayM4_FreePort(m_nPort); m_nPort = -1; }
    if (m_lUserID >= 0) { NET_DVR_Logout(m_lUserID); m_lUserID = -1; }
    NET_DVR_Cleanup();
}

void HikCamera::MockLoop() {
    while (m_mockRunning) {
        PushErrorFrame("HIK: LOGIN FAIL");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void CALLBACK HikCamera::RealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser) {
    HikCamera* pThis = (HikCamera*)pUser;
    if (dwDataType == NET_DVR_SYSHEAD) {
        if (!PlayM4_GetPort(&pThis->m_nPort)) return;
        if (dwBufSize > 0) {
            g_hikDecInst = pThis;
            PlayM4_SetStreamOpenMode(pThis->m_nPort, STREAME_REALTIME);
            PlayM4_OpenStream(pThis->m_nPort, pBuffer, dwBufSize, 1024 * 1024);
#ifdef _WIN32
            PlayM4_SetDecCallBackMend(pThis->m_nPort, DecCallBack, 0);
#else
            PlayM4_SetDecCallBackMend(pThis->m_nPort, DecCallBack, pUser);
#endif
            PlayM4_Play(pThis->m_nPort, 0);
        }
    } else if (dwDataType == NET_DVR_STREAMDATA && pThis->m_nPort != -1 && dwBufSize > 0) {
        PlayM4_InputData(pThis->m_nPort, pBuffer, dwBufSize);
    }
}

#ifdef _WIN32
void CALLBACK HikCamera::DecCallBack(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nUser, long nReserved2) {
    HikCamera* pThis = g_hikDecInst; // Win64 下 nUser(long) 无法承载 64 位指针，改用静态实例
    if(pThis) pThis->ProcessDecodedFrame(pBuf, (int)nSize, pFrameInfo);
}
#else
void CALLBACK HikCamera::DecCallBack(int nPort, char * pBuf, int nSize, FRAME_INFO * pFrameInfo, void* nUser, int nReserved2) {
    HikCamera* pThis = (HikCamera*)nUser;
    if(pThis) pThis->ProcessDecodedFrame(pBuf, nSize, pFrameInfo);
}
#endif

void HikCamera::ProcessDecodedFrame(char* pBuf, int nSize, FRAME_INFO* pFrameInfo) {
    if (pFrameInfo->nType == T_YV12) {
        cv::Mat yv12(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
        cv::Mat bgr;
        cv::cvtColor(yv12, bgr, cv::COLOR_YUV2BGR_YV12);
        PushFrame(bgr);
    }
}
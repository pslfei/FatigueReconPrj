#include "DvtCamera.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <algorithm>

namespace {
constexpr int DVT_HEAD_SIZE = 28;

bool HasAnnexBStartCode(const unsigned char* data, int length, int offset) {
    if (!data || offset < 0 || length - offset < 3) return false;
    if (data[offset] != 0x00 || data[offset + 1] != 0x00) return false;
    if (data[offset + 2] == 0x01) return true;
    return length - offset >= 4 && data[offset + 2] == 0x00 && data[offset + 3] == 0x01;
}
}

ICamera* CreateDvtCamera() { return new DvtCamera(); }
std::atomic<DvtCamera*> DvtCamera::s_activeInstance{nullptr};

DvtCamera::DvtCamera()
    : m_hLogin(nullptr), m_hPreview(nullptr), m_decoder(std::make_unique<FfmpegDecoder>()),
      m_channel(0), m_stopping(false), m_decoderResetPending(false) {}

DvtCamera::~DvtCamera() { 
    Stop();
}

void DvtCamera::SetRecoveryConfig(bool enabled, int frameTimeoutMs, int sdkReconnectIntervalMs, int hardRecoveryTimeoutMs) {
    ICamera::SetRecoveryConfig(enabled, frameTimeoutMs, sdkReconnectIntervalMs, hardRecoveryTimeoutMs);
    ApplySdkReconnectConfig();
}

void DvtCamera::ApplySdkReconnectConfig() {
    const int intervalSeconds = std::max(1, (m_sdkReconnectIntervalMs.load() + 999) / 1000);
    NSDNET_EnableReconnect(m_reconnectEnabled.load() ? 1 : 0, static_cast<unsigned short>(std::min(intervalSeconds, 65535)));
}

bool DvtCamera::InitInternal(const std::string& ip, const std::string& user, const std::string& pass, int channel) {
    std::cout << "--- [DVT] InitInternal ---" << std::endl;
    std::cout << "IP: [" << ip << "] User: [" << user << "]" << std::endl;
    std::cout << "Port: 60000" << std::endl;
    m_channel = std::max(0, channel);
    m_stopping.store(false);
    m_decoderResetPending.store(false);

    NotifyStatus(STATUS_CONNECTING, "Connecting to DVT...");
    if (!m_decoder->init()) {
        NotifyStatus(STATUS_ERROR_DEVICE, "FFmpeg Init Failed");
        return false;
    }
    s_activeInstance.store(this);
    int callbackResult = NSDNET_SetEventCallback(EventCallback);
    if (callbackResult != 0) {
        std::cerr << "[DVT] NSDNET_SetEventCallback failed (" << callbackResult << ")" << std::endl;
    }
    ApplySdkReconnectConfig();

    int ret = NSDNET_Login(ip.c_str(), 60000, user.c_str(), pass.c_str(), 0, this, &m_hLogin);
    if (ret != 0 || !m_hLogin) {
        std::cerr << "[DVT] Login Failed (" << ret << ")." << std::endl;
        NotifyStatus(STATUS_ERROR_AUTH, "DVT Login failed");
        DvtCamera* expected = this;
        s_activeInstance.compare_exchange_strong(expected, nullptr);
        return false;
    }
    NotifyStatus(STATUS_CONNECTING, "DVT login succeeded; waiting for preview");
    return true;
}

bool DvtCamera::Start() {
    if (!m_hLogin) return false;
    m_stopping.store(false);
    NSD_OPEN_CHANNEL_INFO info = {0};
    info.nOpenChannelNo = static_cast<unsigned short>(m_channel);
    info.nSubFlow = 1; 
    info.nTransProtocol = 1; 
    info.funcStramCallback = MediaDataCallback;
    info.lpChannelContext = this;
    int ret = NSDNET_StartPreview(m_hLogin, &info, &m_hPreview);
    if (ret != 0 || !m_hPreview) {
        NotifyStatus(STATUS_ERROR_DEVICE, "DVT preview start failed");
        return false;
    }
    NotifyStatus(STATUS_WAITING_FRAME, "DVT preview started; waiting for first frame");
    return true;
}
void DvtCamera::Stop() {
    if (m_stopping.exchange(true)) return;
    m_streamAvailable.store(false);
    DvtCamera* expected = this;
    s_activeInstance.compare_exchange_strong(expected, nullptr);
    if (m_hPreview) { NSDNET_StopPreview(m_hPreview); m_hPreview = nullptr; }
    if (m_hLogin) { NSDNET_Logout(m_hLogin); m_hLogin = nullptr; }
    {
        std::lock_guard<std::mutex> lock(m_decoderMutex);
        if (m_decoder) m_decoder->reset();
    }
    NotifyStatus(STATUS_DISCONNECTED, "DVT camera stopped");
}

int WINAPI DvtCamera::EventCallback(WPARAM wParam, LPARAM lParam) {
    DvtCamera* camera = s_activeInstance.load();
    if (!camera || camera->m_stopping.load() || !camera->EventBelongsToThis(wParam, lParam)) return 0;
    camera->HandleSdkEvent(wParam, lParam);
    return 0;
}

bool DvtCamera::EventBelongsToThis(WPARAM eventType, LPARAM eventData) const {
    if (!eventData) return true;
    if (eventType == NSDEVENT_PREVIEW_OPEN || eventType == NSDEVENT_PREVIEW_CLOSE) {
        const auto* info = reinterpret_cast<const NSD_EVENT_CHANNEL_INFO*>(eventData);
        return info->lpChannelContext == this;
    }
    if (eventType == NSDEVENT_LOGON_OPEN || eventType == NSDEVENT_LOGON_CLOSE) {
        const auto* info = reinterpret_cast<const NSD_EVENT_LOGON_INFO*>(eventData);
        return info->lpLogonContext == this;
    }
    return true;
}

void DvtCamera::HandleSdkEvent(WPARAM eventType, LPARAM) {
    switch (eventType) {
        case NSDEVENT_LOGON_CLOSE:
        case NSDEVENT_PREVIEW_CLOSE:
            m_decoderResetPending.store(true);
            MarkStreamUnavailable(STATUS_RECONNECTING, "DVT SDK is reconnecting");
            break;
        case NSDEVENT_LOGON_OPEN:
            MarkTransportRecovered("DVT login connection recovered; waiting for preview");
            break;
        case NSDEVENT_PREVIEW_OPEN:
            MarkTransportRecovered("DVT preview recovered; waiting for video frame");
            break;
        default:
            break;
    }
}
#ifdef _WIN32
int WINAPI DvtCamera::MediaDataCallback(HANDLE, int, int, const char* pStreamData, unsigned long nDataLen, void* ctx) {
#else
int DvtCamera::MediaDataCallback(void*, int, int, const char* pStreamData, unsigned long nDataLen, void* ctx) {
#endif
    DvtCamera* pThis = (DvtCamera*)ctx;
    if (pThis && !pThis->m_stopping.load() && pStreamData && nDataLen > 0)
        pThis->ProcessStreamData(pStreamData, static_cast<int>(nDataLen));
    return 0;
}
void DvtCamera::ProcessStreamData(const char* pData, int nLen) {
    if (!pData || nLen < 3 || m_stopping.load()) return;
    std::lock_guard<std::mutex> lock(m_decoderMutex);
    if (m_decoderResetPending.exchange(false)) m_decoder->reset();
    const auto* raw = reinterpret_cast<const unsigned char*>(pData);
    const unsigned char* stream_data = nullptr;
    int stream_length = 0;

    if (HasAnnexBStartCode(raw, nLen, DVT_HEAD_SIZE)) {
        stream_data = raw + DVT_HEAD_SIZE;
        stream_length = nLen - DVT_HEAD_SIZE;
    } else if (HasAnnexBStartCode(raw, nLen, 0)) {
        stream_data = raw;
        stream_length = nLen;
    }
    if (!stream_data) return;

    cv::Mat img;
    if (m_decoder->decode(stream_data, stream_length, img) && !img.empty()) PushFrame(img);
}

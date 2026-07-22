#include "DvtCamera.h"
#include <iostream>
#include <iomanip>
#include <thread>

const int DVT_HEAD_SIZE = 28;

// [诊断函数] (重复定义无妨，这是编译单元内部)
static void HexDump(const char* label, const char* str) {
    if (!str) { std::cout << label << ": [NULL]" << std::endl; return; }
    std::cout << label << ": [" << str << "] Hex: ";
    const unsigned char* p = (const unsigned char*)str;
    while (*p) {
        printf("%02X ", *p);
        p++;
    }
    printf("\n");
}

ICamera* CreateDvtCamera() { return new DvtCamera(); }

DvtCamera::DvtCamera() : m_hLogin(nullptr), m_hPreview(nullptr), m_decoder(nullptr) {
    m_decoder = new FfmpegDecoder();
}

DvtCamera::~DvtCamera() { 
    Stop(); 
    if (m_decoder) delete m_decoder;
}

bool DvtCamera::InitInternal(const std::string& ip, const std::string& user, const std::string& pass, int channel) {
    // [诊断]
    std::cout << "--- [DVT] InitInternal ---" << std::endl;
    HexDump("IP", ip.c_str());
    HexDump("User", user.c_str());
    HexDump("Pass", pass.c_str());
    std::cout << "Port: 60000" << std::endl;

    NotifyStatus(STATUS_CONNECTING, "Connecting to DVT...");
    if (!m_decoder->init()) {
        NotifyStatus(STATUS_ERROR_DEVICE, "FFmpeg Init Failed");
        return true; 
    }
    int ret = NSDNET_Login((char*)ip.c_str(), 60000, (char*)user.c_str(), (char*)pass.c_str(), 0, NULL, &m_hLogin);
    if (ret != 0 || !m_hLogin) {
        std::cerr << "[DVT] Login Failed (" << ret << "). Switched to Mock." << std::endl;
        NotifyStatus(STATUS_ERROR_AUTH, "DVT Login failed");
        return true;
    }
    NotifyStatus(STATUS_CONNECTED, "Connected");
    return true;
}

// ... (Start, Stop, Callback 保持不变) ...
bool DvtCamera::Start() {
    if (!m_hLogin) {
        std::thread([this](){
            while(true) {
                PushErrorFrame("DVT CONNECT ERROR");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }).detach(); 
        return true;
    }
    NSD_OPEN_CHANNEL_INFO info = {0};
    info.nSubFlow = 1; 
    info.nTransProtocol = 1; 
    info.funcStramCallback = MediaDataCallback;
    info.lpChannelContext = this;
    int ret = NSDNET_StartPreview(m_hLogin, &info, &m_hPreview);
    return (ret == 0);
}
void DvtCamera::Stop() {
    if (m_hPreview) { NSDNET_StopPreview(m_hPreview); m_hPreview = nullptr; }
    if (m_hLogin) { NSDNET_Logout(m_hLogin); m_hLogin = nullptr; }
}
#ifdef _WIN32
int WINAPI DvtCamera::MediaDataCallback(HANDLE, int, int, const char* pStreamData, unsigned long nDataLen, void* ctx) {
#else
int DvtCamera::MediaDataCallback(void*, int, int, const char* pStreamData, unsigned long nDataLen, void* ctx) {
#endif
    DvtCamera* pThis = (DvtCamera*)ctx;
    if (pThis && nDataLen > 0) pThis->ProcessStreamData(pStreamData, nDataLen);
    return 0;
}
void DvtCamera::ProcessStreamData(const char* pData, int nLen) {
    const unsigned char* raw = (const unsigned char*)pData;
    if (nLen > DVT_HEAD_SIZE) {
        if (raw[DVT_HEAD_SIZE] == 0x00 && raw[DVT_HEAD_SIZE+1] == 0x00 && 
            raw[DVT_HEAD_SIZE+2] == 0x00 && raw[DVT_HEAD_SIZE+3] == 0x01) {
            cv::Mat img;
            if (m_decoder->decode(raw + DVT_HEAD_SIZE, nLen - DVT_HEAD_SIZE, img)) {
                if (!img.empty()) PushFrame(img);
            }
            return;
        }
    }
    if (raw[0] == 0x00 && raw[1] == 0x00 && raw[2] == 0x00 && raw[3] == 0x01) {
        cv::Mat img;
        if (m_decoder->decode(raw, nLen, img)) {
            if (!img.empty()) PushFrame(img);
        }
    }
}
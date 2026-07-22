#pragma once
#include "../common/ICamera.h"
#ifdef _WIN32
  // DVT 头文件 Windows 分支依赖 windows.h 提供 WINAPI/HANDLE/WPARAM/BOOL 等类型
  #ifndef NOMINMAX
  #define NOMINMAX
  #endif
  #include <windows.h>
#endif
#ifdef BOOL
#undef BOOL
#endif
#include "nsdnetinterface.h"
#include "FfmpegDecoder.h"

class DvtCamera : public ICamera {
public:
    DvtCamera();
    ~DvtCamera();
    
    bool Init(const std::string& ip, const std::string& user, const std::string& pass, int channel, int w, int h) override {
        ICamera::Init(ip, user, pass, channel, w, h);
        return InitInternal(ip, user, pass, channel);
    }
    bool InitInternal(const std::string& ip, const std::string& user, const std::string& pass, int channel);
    
    // [修复] 移除 override
    bool Init(const std::string& ip, const std::string& user, const std::string& pass, int channel) {
        return Init(ip, user, pass, channel, 0, 0);
    }

    bool Start() override;
    void Stop() override;
private:
#ifdef _WIN32
    static int WINAPI MediaDataCallback(HANDLE hOpenChannel, int nChannelNo, int nSubFlow, const char* pStreamData, unsigned long nDataLen, void* lpChannelContext);
#else
    static int MediaDataCallback(void* hOpenChannel, int nChannelNo, int nSubFlow, const char* pStreamData, unsigned long nDataLen, void* lpChannelContext);
#endif
    void ProcessStreamData(const char* pData, int nLen);
    void* m_hLogin;
    void* m_hPreview;
    FfmpegDecoder* m_decoder;
};
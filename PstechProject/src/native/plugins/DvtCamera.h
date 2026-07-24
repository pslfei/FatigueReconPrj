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
#include <atomic>
#include <memory>
#include <mutex>

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
    void SetRecoveryConfig(bool enabled, int frameTimeoutMs, int sdkReconnectIntervalMs, int hardRecoveryTimeoutMs) override;
private:
    static int WINAPI EventCallback(WPARAM wParam, LPARAM lParam);
#ifdef _WIN32
    static int WINAPI MediaDataCallback(HANDLE hOpenChannel, int nChannelNo, int nSubFlow, const char* pStreamData, unsigned long nDataLen, void* lpChannelContext);
#else
    static int MediaDataCallback(void* hOpenChannel, int nChannelNo, int nSubFlow, const char* pStreamData, unsigned long nDataLen, void* lpChannelContext);
#endif
    void ProcessStreamData(const char* pData, int nLen);
    void HandleSdkEvent(WPARAM eventType, LPARAM eventData);
    bool EventBelongsToThis(WPARAM eventType, LPARAM eventData) const;
    void ApplySdkReconnectConfig();
    void* m_hLogin;
    void* m_hPreview;
    std::unique_ptr<FfmpegDecoder> m_decoder;
    int m_channel;
    std::atomic<bool> m_stopping;
    std::atomic<bool> m_decoderResetPending;
    std::mutex m_decoderMutex;
    static std::atomic<DvtCamera*> s_activeInstance;
};

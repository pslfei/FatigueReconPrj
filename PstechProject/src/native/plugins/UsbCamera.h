#pragma once
#include "../common/ICamera.h"
#include <thread>
#include <atomic>

class UsbCamera : public ICamera {
public:
    UsbCamera();
    ~UsbCamera();
    
    bool Init(const std::string&, const std::string&, const std::string&, int index, int w, int h) override;
    
    // [修复] 移除 override
    bool Init(const std::string& c, const std::string& u, const std::string& p, int i) {
        return Init(c, u, p, i, 640, 480);
    }

    bool Start() override;
    void Stop() override;
private:
    void CaptureLoop();
    bool OpenCamera();
    cv::VideoCapture m_cap;
    std::thread m_thread;
    std::atomic<bool> m_running;
    
    // [修复] 补全缺失的成员变量
    int m_deviceIndex; // 对应 cpp 中的 m_deviceIndex
    int m_idx;         // 对应 cpp 报错的 m_idx (统一用 m_idx 吧)
    bool m_isMockMode;
};
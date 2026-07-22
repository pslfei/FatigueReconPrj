#pragma once
#include "../common/ICamera.h"
#include <thread>
#include <atomic>

class VirtualCamera : public ICamera {
public:
    VirtualCamera();
    ~VirtualCamera();
    
    bool Init(const std::string& path, const std::string&, const std::string&, int, int w, int h) override;
    
    // [修复] 移除 override
    bool Init(const std::string& path, const std::string& u, const std::string& p, int i) {
        return Init(path, u, p, i, 640, 480);
    }

    bool Start() override;
    void Stop() override;
private:
    void CaptureLoop();
    cv::VideoCapture m_cap;
    std::thread m_thread;
    std::atomic<bool> m_running;
    
    // [修复] 补全成员
    std::string m_filePath;
};
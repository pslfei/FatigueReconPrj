#include "VirtualCamera.h"
#include <iostream>
ICamera* CreateVirtualCamera() { return new VirtualCamera(); }
VirtualCamera::VirtualCamera() : m_running(false) {}
VirtualCamera::~VirtualCamera() { Stop(); }

bool VirtualCamera::Init(const std::string& path, const std::string& u, const std::string& p, int idx, int w, int h) {
    ICamera::Init(path, u, p, idx, w, h); // 保存宽高
    m_filePath = path;
    return true; // 始终返回成功，具体在 Start/Loop 中处理
}
bool VirtualCamera::Start() {
    m_running = true;
    m_thread = std::thread(&VirtualCamera::CaptureLoop, this);
    return true;
}
void VirtualCamera::Stop() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
    if (m_cap.isOpened()) m_cap.release();
}
void VirtualCamera::CaptureLoop() {
    cv::Mat frame;
    // 首次尝试打开
    m_cap.open(m_filePath, cv::CAP_FFMPEG);
    
    while (m_running) {
        if (!m_cap.isOpened()) {
             // 失败 -> Mock
             PushErrorFrame("NO SIGNAL: File Not Found");
             // 尝试重连
             m_cap.open(m_filePath, cv::CAP_FFMPEG);
             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
             continue;
        }
        if (m_cap.read(frame)) {
            PushFrame(frame);
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        } else {
            m_cap.set(cv::CAP_PROP_POS_FRAMES, 0); // 循环
        }
    }
}
#include "UsbCamera.h"
#include <iostream>
#include <future>
#include <chrono>
#ifndef _WIN32
#include <unistd.h>
#include <sys/stat.h>
#endif

ICamera* CreateUsbCamera() { return new UsbCamera(); }

UsbCamera::UsbCamera() : m_running(false), m_idx(0), m_isMockMode(false) {}
UsbCamera::~UsbCamera() { Stop(); }

#ifdef _WIN32
// Windows 无 /dev/video 设备节点：是否存在交由 OpenCamera 实际尝试打开判断
bool CheckDeviceExist(int /*index*/) {
    return true;
}
#else
bool CheckDeviceExist(int index) {
    std::string devPath = "/dev/video" + std::to_string(index);
    struct stat buffer;
    return (stat(devPath.c_str(), &buffer) == 0);
}
#endif

bool UsbCamera::OpenCamera() {
    if (m_cap.isOpened()) return true;
    if (!CheckDeviceExist(m_idx)) return false;

    std::cout << "[UsbCamera] Async Open camera #" << m_idx << "..." << std::endl;
    std::future<bool> result = std::async(std::launch::async, [this]() {
#ifdef _WIN32
        this->m_cap.open(this->m_idx, cv::CAP_DSHOW);
#else
        this->m_cap.open(this->m_idx, cv::CAP_V4L2);
#endif
        if (this->m_cap.isOpened()) {
            this->m_cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
            this->m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
            this->m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
            this->m_cap.set(cv::CAP_PROP_FPS, 30);
        }
        return this->m_cap.isOpened();
    });

    std::future_status status = result.wait_for(std::chrono::seconds(3));
    if (status == std::future_status::timeout) return false;
    else if (status == std::future_status::ready) return result.get();
    return false;
}

bool UsbCamera::Init(const std::string&, const std::string&, const std::string&, int index, int w, int h) {
    ICamera::Init("", "", "", index, w, h);
    m_idx = index;
    NotifyStatus(STATUS_CONNECTING, "Init USB...");
    
    if (OpenCamera()) {
        NotifyStatus(STATUS_CONNECTED, "USB Opened");
        m_isMockMode = false;
        m_cap.release();
    } else {
        std::cout << "[UsbCamera] Init failed. Switched to Error Mode." << std::endl;
        m_isMockMode = true; // 软失败
    }
    return true; // 始终返回 true，保证主程序不退出
}

bool UsbCamera::Start() {
    if (m_running) return true;
    m_running = true;
    m_thread = std::thread(&UsbCamera::CaptureLoop, this);
    return true;
}

void UsbCamera::Stop() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
    if (m_cap.isOpened()) m_cap.release();
}

void UsbCamera::CaptureLoop() {
    cv::Mat frame;
    while (m_running) {
        if (m_isMockMode) {
            // [修复] 纯黑背景 + 错误文字 (去除蓝色圆圈)
            PushErrorFrame("NO USB DEVICE");
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 低帧率省资源 
        } else {
            if (!m_cap.isOpened()) {
                if (!OpenCamera()) {
                    // 运行时断开 -> 切回 Mock
                    m_isMockMode = true;
                    continue;
                }
            }
            if (m_cap.read(frame)) {
                PushFrame(frame);
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
}
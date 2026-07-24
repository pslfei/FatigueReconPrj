#pragma once
#include "LinuxCompat.h"
#include <opencv2/opencv.hpp>
#include "PstechStatus.h"
#include "RingBuffer.h"
#include <algorithm>
#include <chrono>
#include <mutex>

enum CameraType { CAM_VIRTUAL_FILE=0, CAM_USB=1, CAM_HIKVISION=2, CAM_DVT=3 };

class ICamera {
protected:
    RingBuffer m_ringBuffer;
    StatusCallback m_statusCb = nullptr;
    
    int m_targetW = 640;
    int m_targetH = 480;

    std::atomic<int> m_lastFrameState;
    std::atomic<int> m_runtimeStatus{STATUS_DISCONNECTED};
    std::atomic<bool> m_streamAvailable{false};
    std::atomic<long long> m_lastValidFrameSteadyMs{0};
    std::atomic<unsigned long long> m_frameSequence{0};

    std::atomic<bool> m_reconnectEnabled{true};
    std::atomic<int> m_frameTimeoutMs{3000};
    std::atomic<int> m_sdkReconnectIntervalMs{5000};
    std::atomic<int> m_hardRecoveryTimeoutMs{0};

public:
    bool m_checkBlack = true;
    double m_brightnessThreshold = 10.0;
    std::atomic<double> m_lastBrightness{0.0};
    ICamera() : m_lastFrameState(FRAME_EMPTY) {}
    virtual ~ICamera() {}
    
    virtual bool Init(const std::string& connectStr, const std::string& user, const std::string& pass, int indexOrPort, int w, int h) {
        if (w > 0 && h > 0) {
            m_targetW = w;
            m_targetH = h;
        }
        return true; 
    };

    virtual bool Start() = 0;
    virtual void Stop() = 0;

    virtual void SetRecoveryConfig(bool enabled, int frameTimeoutMs, int sdkReconnectIntervalMs, int hardRecoveryTimeoutMs) {
        m_reconnectEnabled.store(enabled);
        m_frameTimeoutMs.store(std::clamp(frameTimeoutMs, 500, 60000));
        m_sdkReconnectIntervalMs.store(std::clamp(sdkReconnectIntervalMs, 1000, 300000));
        m_hardRecoveryTimeoutMs.store(std::clamp(hardRecoveryTimeoutMs, 0, 600000));
    }

    void SetStatusCallback(StatusCallback cb) { m_statusCb = cb; }
    void NotifyStatus(int status, const std::string& msg) {
        m_runtimeStatus.store(status);
        if (m_statusCb) m_statusCb(status, msg.c_str());
    }

    virtual int GetFrameState(unsigned char** ptr, int& w, int& h, long long& ts) {
        int nodeState = FRAME_EMPTY;
        *ptr = m_ringBuffer.GetLatest(w, h, ts, &nodeState);
        if (*ptr == nullptr) return FRAME_EMPTY;

        if (!m_streamAvailable.load()) return FRAME_BLACK_SCREEN;

        const long long lastValid = m_lastValidFrameSteadyMs.load();
        const long long now = SteadyNowMs();
        if (lastValid > 0 && now - lastValid >= m_frameTimeoutMs.load()) {
            bool wasAvailable = m_streamAvailable.exchange(false);
            if (wasAvailable) NotifyStatus(STATUS_RECONNECTING, "Video frame timeout; waiting for SDK reconnect");
            return FRAME_BLACK_SCREEN;
        }
        return nodeState;
    }

    // Compat
    virtual unsigned char* GetLatestFrame(int& w, int& h, long long& ts) { return m_ringBuffer.GetLatest(w, h, ts); }
    virtual unsigned char* GetRawBuffer(int& w, int& h, long long& ts) { return GetLatestFrame(w, h, ts); }
    virtual bool TryLockBuffer() { return true; }
    virtual void LockBuffer() { } 
    virtual void UnlockBuffer() { }
    virtual bool GetFrame(cv::Mat& outFrame) {
        int w, h; long long ts;
        unsigned char* ptr = GetLatestFrame(w, h, ts);
        if (!ptr) return false;
        outFrame = cv::Mat(h, w, CV_8UC3);
        memcpy(outFrame.data, ptr, w*h*3);
        return true;
    }

protected:
    static long long SteadyNowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    void MarkStreamUnavailable(int status, const std::string& message) {
        m_streamAvailable.store(false);
        NotifyStatus(status, message);
    }

    void MarkTransportRecovered(const std::string& message) {
        m_streamAvailable.store(false);
        NotifyStatus(STATUS_WAITING_FRAME, message);
    }

    void PushErrorFrame(const std::string& errorMsg) {
        cv::Mat black(m_targetH, m_targetW, CV_8UC3, cv::Scalar(10, 10, 10)); 
        int baseline = 0;
        cv::Size textSize = cv::getTextSize(errorMsg, cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);
        cv::Point textOrg((m_targetW - textSize.width) / 2, (m_targetH + textSize.height) / 2);
        cv::putText(black, errorMsg, textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
        PushFrame(black, true); 
    }

    void PushFrame(const cv::Mat& frame, bool isForcedError = false) {
        if (frame.empty()) return;
        
        cv::Mat resized;
        // [核心确认] 使用 cv::resize 进行缩放，而不是截取
        if (frame.cols != m_targetW || frame.rows != m_targetH) {
            // 调试日志 (仅前几次打印)
            static int resizeLog = 0;
            if (resizeLog++ < 5) {
                std::cout << "[Camera] Resizing " << frame.cols << "x" << frame.rows 
                          << " -> " << m_targetW << "x" << m_targetH << std::endl;
            }
            cv::resize(frame, resized, cv::Size(m_targetW, m_targetH));
        } else {
            resized = frame;
        }

        int frameState = FRAME_OK;
        if (isForcedError) {
            frameState = FRAME_BLACK_SCREEN;
        }
        else if (m_checkBlack) {
            cv::Scalar meanVal = cv::mean(resized);
            double brightness = (meanVal[0] + meanVal[1] + meanVal[2]) / 3.0;
            m_lastBrightness.store(brightness);
            frameState = (brightness < m_brightnessThreshold) ? FRAME_BLACK_SCREEN : FRAME_OK;
        } else {
            frameState = FRAME_OK;
        }

        m_lastFrameState.store(frameState);

        long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        const unsigned long long sequence = m_frameSequence.fetch_add(1) + 1;
        m_ringBuffer.Write(resized.data, resized.cols, resized.rows, ts, frameState, sequence);

        // 错误占位图不是摄像头产生的有效新帧，不能据此宣布连接恢复。
        if (!isForcedError) {
            m_lastValidFrameSteadyMs.store(SteadyNowMs());
            const bool wasAvailable = m_streamAvailable.exchange(true);
            const int previousStatus = m_runtimeStatus.exchange(STATUS_CONNECTED);
            if (!wasAvailable || previousStatus != STATUS_CONNECTED) {
                if (m_statusCb) m_statusCb(STATUS_CONNECTED, "Video stream is producing frames");
            }
        }
    }
};

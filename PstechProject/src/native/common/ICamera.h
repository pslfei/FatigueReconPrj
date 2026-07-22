#pragma once
#include "LinuxCompat.h"
#include <opencv2/opencv.hpp>
#include "PstechStatus.h"
#include "RingBuffer.h"

enum CameraType { CAM_VIRTUAL_FILE=0, CAM_USB=1, CAM_HIKVISION=2, CAM_DVT=3 };

class ICamera {
protected:
    RingBuffer m_ringBuffer;
    StatusCallback m_statusCb = nullptr;
    
    int m_targetW = 640;
    int m_targetH = 480;

    std::atomic<int> m_lastFrameState;

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

    void SetStatusCallback(StatusCallback cb) { m_statusCb = cb; }
    void NotifyStatus(int status, const std::string& msg) { if (m_statusCb) m_statusCb(status, msg.c_str()); }

    virtual int GetFrameState(unsigned char** ptr, int& w, int& h, long long& ts) {
        *ptr = m_ringBuffer.GetLatest(w, h, ts);
        if (*ptr == nullptr) return FRAME_EMPTY;
        return m_lastFrameState.load();
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

        if (isForcedError) {
            m_lastFrameState.store(FRAME_BLACK_SCREEN);
        }
        else if (m_checkBlack) {
            cv::Scalar meanVal = cv::mean(resized);
            double brightness = (meanVal[0] + meanVal[1] + meanVal[2]) / 3.0;
            m_lastBrightness.store(brightness);
            m_lastFrameState.store((brightness < m_brightnessThreshold) ? FRAME_BLACK_SCREEN : FRAME_OK);
        } else {
            m_lastFrameState.store(FRAME_OK);
        }

        long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        m_ringBuffer.Write(resized.data, resized.cols, resized.rows, ts);
    }
};
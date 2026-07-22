#include "../pstrack/PsTrackWrapper.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>

#define PI 3.14159265

// 定义版本字符串，包含编译时间
#define DEMO_VERSION "PsTrack Demo v0.48 (Build: " __DATE__ " " __TIME__ ")"

void Draw3DAxes(cv::Mat& img, cv::Point center, float pitch, float yaw, float roll, float scale) {
    // 投影计算
    float x1 = scale * cos(yaw) * cos(roll);
    float y1 = scale * (cos(pitch) * sin(roll) + cos(roll) * sin(pitch) * sin(yaw));
    float x2 = -scale * cos(yaw) * sin(roll);
    float y2 = -scale * (cos(pitch) * cos(roll) - sin(pitch) * sin(yaw) * sin(roll));
    float x3 = scale * sin(yaw);
    float y3 = -scale * sin(pitch) * cos(yaw);
    
    // OpenCV Y轴向下，需反转 y 分量
    cv::Point pX = center + cv::Point((int)x1, (int)(-y1)); 
    cv::Point pY = center + cv::Point((int)x2, (int)(-y2)); 
    cv::Point pZ = center + cv::Point((int)x3, (int)(-y3)); 
    
    cv::arrowedLine(img, center, pX, cv::Scalar(0, 0, 255), 2); // X - Red (左右)
    cv::arrowedLine(img, center, pY, cv::Scalar(0, 255, 0), 2); // Y - Green (上下)
    cv::arrowedLine(img, center, pZ, cv::Scalar(255, 0, 0), 2); // Z - Blue (朝向)
}

void DrawVisuals(cv::Mat& img, const PsTrackFaceData& data, long timeMs) {
    // 1. 绘制特征点
    for(int i=0; i < data.landmarkCount; ++i) {
        float rx = data.landmarks[i*2];
        float ry = data.landmarks[i*2+1];
        int px = (int)(rx * img.cols);
        int py = (int)((1.0f - ry) * img.rows);
        cv::circle(img, cv::Point(px, py), 1, cv::Scalar(0, 255, 255), -1);
    }
    
    // 2. 绘制 3D 轴 (基于鼻尖)
    float cx = data.origin[0] * img.cols;
    float cy = (1.0f - data.origin[1]) * img.rows;
    if (cx > 0 && cy > 0) {
        Draw3DAxes(img, cv::Point((int)cx, (int)cy), data.headPose[0], data.headPose[1], data.headPose[2], 80.0f);
    }
    
    // 3. 绘制文字信息
    std::vector<std::string> lines;
    lines.push_back("System: PsTrack Engine");
    lines.push_back("Proc Time: " + std::to_string(timeMs) + " ms");
    
    float r2d = 180.0f / PI;
    char buff[100];
    snprintf(buff, sizeof(buff), "Pose: P=%d Y=%d R=%d", 
        (int)(data.headPose[0]*r2d), (int)(data.headPose[1]*r2d), (int)(data.headPose[2]*r2d));
    lines.push_back(buff);
    
    // 眼睛状态 (值大为开)
    bool lOpen = data.eyeClosure[0] > 0.5f;
    bool rOpen = data.eyeClosure[1] > 0.5f;
    lines.push_back(std::string("L: ") + (lOpen ? "OPEN" : "CLOSED"));
    lines.push_back(std::string("R: ") + (rOpen ? "OPEN" : "CLOSED"));

    int y = 30;
    for (const auto& line : lines) {
        cv::putText(img, line, cv::Point(21, y+1), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0,0,0), 2);
        cv::putText(img, line, cv::Point(20, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255,255,255), 1);
        y += 30;
    }

    // 4. [新增] 绘制右下角版本号水印 (确信是最新版)
    cv::putText(img, DEMO_VERSION, cv::Point(img.cols - 450, img.rows - 10), 
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
}

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0); 
    
    if (argc < 2) {
        std::cerr << "Usage: ./Test_PsTrack <config> [video]" << std::endl;
        return -1;
    }

    PsTrackWrapper tracker;
    tracker.Init(argv[1]);

    std::string vidPath = (argc > 2) ? argv[2] : "test.mp4";
    cv::VideoCapture cap;
    cap.open(vidPath, cv::CAP_FFMPEG);
    
    std::string winName = "PsTrack Monitor";
    cv::namedWindow(winName, cv::WINDOW_NORMAL);
    cv::resizeWindow(winName, 960, 720);

    cv::Mat frame;
    PsTrackFaceData data;
    
    while(true) {
        if (cap.isOpened()) {
            if (!cap.read(frame)) { cap.set(cv::CAP_PROP_POS_FRAMES, 0); continue; }
        } else {
            frame = cv::Mat(480, 640, CV_8UC3, cv::Scalar(40,40,40));
            cv::putText(frame, "NO VIDEO", cv::Point(200,240), 0, 1, cv::Scalar(0,0,255), 2);
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
        
        auto t1 = std::chrono::high_resolution_clock::now();
        tracker.Track(frame.data, frame.cols, frame.rows, &data);
        auto t2 = std::chrono::high_resolution_clock::now();
        
        if (data.isDetected) {
            long duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            DrawVisuals(frame, data, duration);
        } else {
            cv::putText(frame, "NO FACE DETECTED", cv::Point(50, 400), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0,0,255), 2);
            // 也要画水印
            cv::putText(frame, DEMO_VERSION, cv::Point(frame.cols - 450, frame.rows - 10), 
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);
        }
        
        cv::imshow(winName, frame);
        if (cv::waitKey(30) == 27) break;
    }
    return 0;
}
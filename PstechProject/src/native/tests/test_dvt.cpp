#include "../plugins/CameraFactory.h"
#include <iostream>
#include <opencv2/highgui.hpp>
#include <thread>
void OnStatus(int s, const char* msg) { std::cout << "[STATUS] " << s << ": " << msg << std::endl; }
int main(int argc, char** argv) {
    if (argc < 5) return -1;
    ICamera* cam = CreateDvtCamera();
    cam->SetStatusCallback(OnStatus);
    // [修复] 传入 640x480
    if (!cam->Init(argv[1], argv[2], argv[3], atoi(argv[4]), 640, 480)) return -1;
    cam->Start();
    cv::namedWindow("Monitor", cv::WINDOW_NORMAL);
    while(true) {
        int w,h; long long ts;
        unsigned char* buf = cam->GetLatestFrame(w,h,ts);
        if(buf) {
            cv::Mat f(h,w,CV_8UC3, buf);
            cv::imshow("Monitor", f);
        }
        if(cv::waitKey(30)==27) break;
    }
    cam->Stop();
    return 0;
}
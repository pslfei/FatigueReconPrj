#pragma once
#include <vector>

struct PsTrackFaceData {
    int isDetected;
    float eyeClosure[2]; 
    float headPose[3];
    float faceRect[4];
    float origin[2];     // 鼻尖
    int landmarkCount;
    float* landmarks; 
};

class PsTrackWrapper {
public:
    PsTrackWrapper();
    ~PsTrackWrapper();
    void Init(const char* cfgPath);
    void Track(unsigned char* imageData, int width, int height, PsTrackFaceData* outData);
private:
    void* m_tracker; 
    void* m_trackingData; 
    std::vector<float> m_landmarkBuffer;
};
#include "PsTrackWrapper.h"
#include <cstring>
#include <iostream>
#include <limits.h>
#include <stdlib.h>

#ifdef __linux__
    #define VISAGE_DECLSPEC 
    #define __declspec(x)
#endif

#include <opencv2/core/types_c.h>
#include "visageVision.h"

PsTrackWrapper::PsTrackWrapper() : m_tracker(nullptr), m_trackingData(nullptr) {
    m_trackingData = new VisageSDK::FaceData();
}

PsTrackWrapper::~PsTrackWrapper() {
    if (m_tracker) delete (VisageSDK::VisageTracker*)m_tracker;
    if (m_trackingData) delete (VisageSDK::FaceData*)m_trackingData;
}

void PsTrackWrapper::Init(const char* cfgPath) {
#ifdef _WIN32
    char absPath[_MAX_PATH];
    const char* finalPath = _fullpath(absPath, cfgPath, _MAX_PATH) ? absPath : cfgPath;
#else
    char absPath[PATH_MAX];
    char* res = realpath(cfgPath, absPath);
    const char* finalPath = (res) ? absPath : cfgPath;
#endif
    
    std::cout << "[PsTrack] Init Engine: " << finalPath << std::endl;
    try {
        m_tracker = new VisageSDK::VisageTracker(finalPath);
    } catch (...) {
        std::cerr << "[PsTrack] Error: Engine init failed." << std::endl;
        m_tracker = nullptr;
    }
}

void PsTrackWrapper::Track(unsigned char* imageData, int width, int height, PsTrackFaceData* outData) {
    if (!m_tracker || !m_trackingData || !imageData) {
        if(outData) outData->isDetected = 0;
        return;
    }
    
    VisageSDK::VisageTracker* tracker = (VisageSDK::VisageTracker*)m_tracker;
    VisageSDK::FaceData* data = (VisageSDK::FaceData*)m_trackingData;

    #ifndef VISAGE_FRAMEGRABBER_FMT_BGR
    #define VISAGE_FRAMEGRABBER_FMT_BGR 0
    #endif
    #ifndef TRACK_STAT_OK
    #define TRACK_STAT_OK 1
    #endif

    try {
        int* status = tracker->track(width, height, (const char*)imageData, data, VISAGE_FRAMEGRABBER_FMT_BGR);
        outData->isDetected = (status[0] == TRACK_STAT_OK);
    } catch (...) {
        outData->isDetected = 0;
    }
    
    if (outData->isDetected) {
        outData->eyeClosure[0] = data->eyeClosure[0];
        outData->eyeClosure[1] = data->eyeClosure[1];
        
        outData->headPose[0] = data->faceRotation[0];
        outData->headPose[1] = data->faceRotation[1];
        outData->headPose[2] = data->faceRotation[2];
        
        // 提取鼻尖 (9.3)
        const VisageSDK::FeaturePoint& noseTip = data->featurePoints2D->getFP(9, 3);
        if (noseTip.defined && noseTip.pos[0] > 0) {
            outData->origin[0] = noseTip.pos[0];
            outData->origin[1] = noseTip.pos[1];
        } else {
            const VisageSDK::FeaturePoint& noseRoot = data->featurePoints2D->getFP(12, 1);
            outData->origin[0] = noseRoot.pos[0];
            outData->origin[1] = noseRoot.pos[1];
        }
        
        // 全量点
        if (data->featurePoints2D) {
            m_landmarkBuffer.clear();
            for (int g = 2; g <= 14; ++g) {
                for (int i = 1; i <= 60; ++i) {
                    const VisageSDK::FeaturePoint& fp = data->featurePoints2D->getFP(g, i);
                    if (fp.defined && (fp.pos[0] > 0.001f || fp.pos[1] > 0.001f)) {
                        m_landmarkBuffer.push_back(fp.pos[0]);
                        m_landmarkBuffer.push_back(fp.pos[1]);
                    }
                }
            }
            outData->landmarkCount = m_landmarkBuffer.size() / 2;
            outData->landmarks = m_landmarkBuffer.data();
        }
    }
}
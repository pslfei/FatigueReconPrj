#include "../common/ICamera.h"
#include "../plugins/CameraFactory.h"
#include "../pstrack/PsTrackWrapper.h"

#ifdef _WIN32
#define PSTECH_API __declspec(dllexport)
#else
#define PSTECH_API __attribute__((visibility("default")))
#endif

static ICamera* g_camera = nullptr;
static PsTrackWrapper* g_pstrack = nullptr;
static StatusCallback g_globalStatusCb = nullptr;

extern "C" {
    void InternalStatusCallback(int status, const char* msg) {
        if (g_globalStatusCb) g_globalStatusCb(status, msg);
    }
    PSTECH_API void Pstech_SetStatusCallback(StatusCallback cb) { g_globalStatusCb = cb; }
    
    // [更新] 增加 w, h
    PSTECH_API int Pstech_Camera_Init(int type, const char* conn, const char* u, const char* p, int idx, int w, int h) {
        if (g_camera) { delete g_camera; g_camera = nullptr; }
        g_camera = CameraFactory::CreateCamera((CameraType)type);
        if (!g_camera) return -1;
        g_camera->SetStatusCallback(InternalStatusCallback);
        // Init 现在总是返回 true (逻辑移到 Start/Loop)
        return g_camera->Init(conn ? conn : "", u ? u : "", p ? p : "", idx, w, h) ? 0 : -2;
    }
    PSTECH_API void Pstech_Camera_SetBlackScreenConfig(bool checkBlack, double brightnessThreshold) {
        if (g_camera) {
            g_camera->m_checkBlack = checkBlack;
            g_camera->m_brightnessThreshold = brightnessThreshold;
        }
    }
    PSTECH_API double Pstech_Camera_GetBrightness() {
        if (g_camera) return g_camera->m_lastBrightness.load();
        return -1.0;
    }
    PSTECH_API int Pstech_Camera_Start() { return (g_camera && g_camera->Start()) ? 0 : -1; }
    PSTECH_API void Pstech_Camera_Stop() { if (g_camera) g_camera->Stop(); }

    PSTECH_API int Pstech_Camera_LockAndGet(unsigned char** ptr, int* w, int* h, long long* ts) {
        if (g_camera) return g_camera->GetFrameState(ptr, *w, *h, *ts);
        return 0;
    }
    PSTECH_API void Pstech_Camera_Unlock() { }

    PSTECH_API void Pstech_Alg_Init(const char* cfgPath) {
        if (!g_pstrack) g_pstrack = new PsTrackWrapper();
        g_pstrack->Init(cfgPath);
    }
    PSTECH_API void Pstech_Alg_Track(unsigned char* imgPtr, int w, int h, PsTrackFaceData* outData) {
        if (g_pstrack && imgPtr) g_pstrack->Track(imgPtr, w, h, outData);
    }
}
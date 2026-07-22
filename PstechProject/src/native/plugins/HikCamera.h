#pragma once
#include "../common/ICamera.h"
#include "HCNetSDK.h"
#ifdef __linux__
#include "LinuxPlayM4.h"
#else
#include "PlayM4.h"
#endif

#include <thread>
#include <atomic>

class HikCamera : public ICamera {
public:
    HikCamera();
    ~HikCamera();
    
    bool Init(const std::string& ip, const std::string& user, const std::string& pass, int port, int w, int h) override {
        ICamera::Init(ip, user, pass, port, w, h);
        return InitInternal(ip, user, pass, port);
    }
    
    bool InitInternal(const std::string& ip, const std::string& user, const std::string& pass, int port);
    
    bool Init(const std::string& ip, const std::string& user, const std::string& pass, int port) {
        return Init(ip, user, pass, port, 0, 0);
    }

    bool Start() override;
    void Stop() override;

private:
    static void CALLBACK RealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
#ifdef _WIN32
    static void CALLBACK DecCallBack(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nUser, long nReserved2);
#else
    static void CALLBACK DecCallBack(int nPort, char * pBuf, int nSize, FRAME_INFO * pFrameInfo, void* nUser, int nReserved2);
#endif
    void ProcessDecodedFrame(char* pBuf, int nSize, FRAME_INFO* pFrameInfo);
    void MockLoop(); // [新增] Mock 线程函数

    LONG m_lUserID;
    LONG m_lRealPlayHandle;
    LONG m_nPort;
    std::string m_ip;
    
    // [新增] 线程控制
    std::thread m_mockThread;
    std::atomic<bool> m_mockRunning;
};
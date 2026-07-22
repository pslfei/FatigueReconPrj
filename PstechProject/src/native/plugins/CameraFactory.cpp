#include "CameraFactory.h"
#include <iostream>

#ifndef TEST_MODE
    #define ENABLE_VIRTUAL
    #define ENABLE_USB
    #define ENABLE_HIK
    #define ENABLE_DVT
#endif

// 各摄像头插件在各自 .cpp 中提供强符号 CreateXxxCamera()
#if defined(_WIN32)
// MSVC 不支持 __attribute__((weak))。Windows 全通路编译，4 个插件强符号齐全，此处仅前向声明
ICamera* CreateVirtualCamera();
ICamera* CreateUsbCamera();
ICamera* CreateHikCamera();
ICamera* CreateDvtCamera();
#else
// GCC/Clang 弱符号：某插件未编入时回退返回 nullptr
ICamera* __attribute__((weak)) CreateVirtualCamera() { return nullptr; }
ICamera* __attribute__((weak)) CreateUsbCamera() { return nullptr; }
ICamera* __attribute__((weak)) CreateHikCamera() { return nullptr; }
ICamera* __attribute__((weak)) CreateDvtCamera() { return nullptr; }
#endif

ICamera* CameraFactory::CreateCamera(CameraType type) {
    switch (type) {
        case CAM_VIRTUAL_FILE: 
            #if defined(ENABLE_VIRTUAL) || defined(TEST_ONLY_FILE)
            return CreateVirtualCamera();
            #else
            return nullptr;
            #endif
        case CAM_USB:          
            #if defined(ENABLE_USB) || defined(TEST_ONLY_USB)
            return CreateUsbCamera();
            #else
            return nullptr;
            #endif
        case CAM_HIKVISION:    
            #if defined(ENABLE_HIK) || defined(TEST_ONLY_HIK)
            return CreateHikCamera();
            #else
            return nullptr;
            #endif
        case CAM_DVT:          
            #if defined(ENABLE_DVT) || defined(TEST_ONLY_DVT)
            return CreateDvtCamera();
            #else
            return nullptr;
            #endif
        default: return nullptr;
    }
}
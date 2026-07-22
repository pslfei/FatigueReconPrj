#pragma once
#include "../common/ICamera.h"

// 纯 C++ 函数声明
ICamera* CreateHikCamera();
ICamera* CreateDvtCamera();
ICamera* CreateUsbCamera();
ICamera* CreateVirtualCamera();

class CameraFactory {
public:
    static ICamera* CreateCamera(CameraType type);
};
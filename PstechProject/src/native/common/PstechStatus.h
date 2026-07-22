#pragma once
enum CameraStatus {
    STATUS_DISCONNECTED = 0,
    STATUS_CONNECTING = 1,
    STATUS_CONNECTED = 2,
    STATUS_ERROR_AUTH = 3,
    STATUS_ERROR_NETWORK = 4,
    STATUS_ERROR_DEVICE = 5
};

// 帧状态 (LockAndGet 返回值)
enum FrameState {
    FRAME_EMPTY = 0,
    FRAME_OK = 1,
    FRAME_BLACK_SCREEN = 2, // 黑屏/遮挡
    FRAME_FROZEN = 3        // 画面卡死(可选)
};

typedef void (*StatusCallback)(int status, const char* msg);
# Windows 库编译可行性评估

> 评估对象：PstechFatigueSystem（威视疲劳检测系统）能否编译输出 Windows 下可用的库
> 评估日期：2026-06-17
> 评估范围：C++ 原生库 `PstechNative`（Windows 下为 `.dll`）+ C# 应用层 + 第三方 SDK
> 调研方式：静态检索 `CMakeLists.txt`、`src/native/` 源码、`sdk/` 目录、C# P/Invoke 层与 `.csproj`

---

## 一、总体结论

**当前状态下不能直接编译出 Windows 库，但项目架构对 Windows 移植友好**——开发者已在多个关键边界预留了 Windows 支持。真正的障碍集中在「构建脚本」与「第三方运行时 DLL」，而非核心业务代码。整体属于**有条件可行、需中等量改造**。

一句话概括：**代码改动量很小，成本主要在凑齐第三方 Windows 运行时库。**

---

## 二、分层就绪度评估

| 层 | 文件 | 就绪度 | 说明 |
|----|------|--------|------|
| C++ 导出层 | `src/native/export/PstechExport.cpp` | 就绪 | 已写 `#ifdef _WIN32 → __declspec(dllexport)` 跨平台导出宏 |
| C# 互操作层 | `src/managed/Pstech.Core/NativeMethods.cs` | 就绪 | `DllName = "PstechNative"`（无 `lib`/`.so`），.NET 自动解析为 `PstechNative.dll`，无需改动 |
| C# 项目配置 | `src/managed/Pstech.App/Pstech.App.csproj` | 就绪 | 仅 `<PublishAot>true</PublishAot>`，未硬编码 `linux-x64`，可直接 `dotnet publish -r win-x64` |
| C++ 业务源码 | `src/native/`（24 个 cpp/h） | 几乎就绪 | 仅 `UsbCamera.cpp` 1 个文件含 Linux 专有调用，其余跨平台 |
| 构建系统 | `CMakeLists.txt` | 需重写 | 完全为 Linux/GCC 硬编码 |
| 第三方运行时库 | `sdk/*/lib/windows/` | 不完整 | 海康/DVT 缺 `.dll`；OpenCV/FFmpeg 需自备 |
| 授权模块（加密狗） | `PstechEncryption` / `jiamigou.lib` | 待确认 | 详见 §四.4 |

---

## 三、有利条件（已具备的 Windows 支持）

### 3.1 C ABI 导出层已跨平台

`src/native/export/PstechExport.cpp` 第 5-9 行已预留 Windows 导出宏：

```cpp
#ifdef _WIN32
#define PSTECH_API __declspec(dllexport)
#else
#define PSTECH_API __attribute__((visibility("default")))
#endif

extern "C" {
    PSTECH_API void Pstech_SetStatusCallback(StatusCallback cb) { ... }
    ...
}
```

说明开发者**在设计阶段就考虑过 Windows 编译**，C ABI 边界无需改动即可在 MSVC 下正确导出符号。

### 3.2 C# P/Invoke 层为跨平台库名

`src/managed/Pstech.Core/NativeMethods.cs` 第 33 行：

```csharp
private const string DllName = "PstechNative";
[DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
```

库名不含 `lib` 前缀与 `.so` 后缀，.NET 运行时会按平台自动解析为 `PstechNative.dll`（Windows）或 `libPstechNative.so`（Linux）。**C# 互操作层零改动。**

### 3.3 C# 项目未锁定 Linux 运行时

`Pstech.App.csproj` 中仅有 `<PublishAot>true</PublishAot>`，没有写死 `RuntimeIdentifier`（根文档中的 `linux-x64` 仅为 `dotnet publish` 命令行参数）。AOT 在 Windows 主机上可直接发布 `win-x64` 目标。**C# 应用层零改动。**

### 3.4 平台耦合代码高度集中

全量检索 `src/native/` 仅 `UsbCamera.cpp` 命中 Linux 专有头文件/系统调用（`unistd.h`、`/dev/video`、`cv::CAP_V4L2`）。其余文件（HikCamera、DvtCamera、VirtualCamera、PsTrackWrapper、ICamera、RingBuffer、CameraFactory、FfmpegDecoder 等）均通过 OpenCV 与标准库实现，**与平台解耦良好**。

---

## 四、必须解决的阻塞项

### 4.1 CMakeLists.txt 全是 Linux 硬编码（必改，工作量中等）

当前构建脚本无法在 Windows/MSVC 下工作，问题点：

- 库路径写死 Linux 子目录：
  ```cmake
  set(HIK_LIB_PATH    "${CMAKE_SOURCE_DIR}/sdk/hikvision/lib/linux")
  set(DVT_LIB_PATH    "${CMAKE_SOURCE_DIR}/sdk/dvt/lib/linux")
  set(VISAGE_LIB_PATH "${CMAKE_SOURCE_DIR}/sdk/visage/lib/linux")
  ```
- 链接目标为 `.so`/`.a`（`libhcnetsdk.so`、`libNsdNetSDK.so`、`libVisageVision.a`），Windows 应链接 `lib/windows` 下的 `.lib`
- `link_directories(/usr/lib/x86_64-linux-gnu /usr/local/lib)`——Linux 专有系统库路径
- GCC 专有编译选项：`-fpermissive -w -O2 -fopenmp`（已在 `if(UNIX)` 内，但 MSVC 分支缺失）
- 第 8 行 `add_definitions(-DLINUX)` 强制定义 `LINUX` 宏，会误导 Visage SDK 头文件的平台分支判断，Windows 下应改为条件定义（如 `if(WIN32) add_definitions(-DWIN32 ...) else() add_definitions(-DLINUX ...)`）
- `-D_GLIBCXX_USE_CXX11_ABI=1` 为 GCC libstdc++ 专有，MSVC 下无意义（无害但应隔离到 UNIX 分支）

改造方向：引入 `if(WIN32)/elseif(UNIX)` 分支，分别设置库路径、链接库、编译选项与平台宏。

### 4.2 UsbCamera.cpp 依赖 Linux V4L2（必改或排除，工作量小）

`src/native/plugins/UsbCamera.cpp`：

```cpp
#include <unistd.h>
std::string devPath = "/dev/video" + std::to_string(index);
m_cap.open(this->m_idx, cv::CAP_V4L2);   // V4L2 为 Linux 专有后端
```

Windows 下 USB 摄像头需改用 `cv::CAP_DSHOW` 或 `cv::CAP_MSMF`，且 `/dev/videoN` 设备路径不适用。

两种方案：
- **方案 A（推荐，若 Windows 部署不需要 USB 摄像头）**：从 Windows 构建中排除该文件
- **方案 B**：加 `#ifdef _WIN32` 条件编译，Windows 走 `CAP_DSHOW`/`CAP_MSMF` 并去掉 `/dev/video` 路径与 `unistd.h`

### 4.3 第三方运行时 DLL 缺失（最大不确定性，依赖外部资源）

`sdk/` 下已预留 `windows` 目录（`sdk/*/lib/windows/README.txt` 明确写明「请放入 Windows 库文件 (.lib / .dll)」），但运行时 DLL 不齐：

| SDK | 编译/链接（.lib） | 运行时（.dll） | 备注 |
|-----|------------------|---------------|------|
| Visage | 齐全：`libVisageVision64.lib`、`libVisageAnalyser64.lib`、`libVisageGaze64.lib` 等 | 待确认 | Linux 侧依赖 OpenVINO 推理引擎（`libinference_engine.so`、`libngraph.so`、`libtbb.so`、`libMKLDNNPlugin.so`、`libOVPlugin.so`），Windows 侧若同样依赖则需对应运行时 |
| Hikvision | 有导入库：`HCNetSDK.lib`、`PlayCtrl.lib`、`HCCore.lib`、`GdiPlus.lib` | 缺失 | 导入库可过编译，但运行时缺 `HCNetSDK.dll`、`PlayCtrl.dll` 等 |
| DVT | 有导入库：`NsdNetSDK_x64.lib` | 缺失 | 同上，运行时缺 `NsdNetSDK.dll` |
| OpenCV | 需自行安装 Windows 开发库并设置 `OpenCV_DIR` | 需随产物分发 | `find_package(OpenCV)` 在 Windows 上依赖本地安装 |
| FFmpeg | DVT 通路依赖（`FfmpegDecoder.h`），需 Windows 开发库 | 需随产物分发 | avcodec/avformat/avutil/swscale |

要点：海康与 DVT 的 Windows `.lib` 是**导入库（import library）**，可让链接通过，但**运行时仍需对应 `.dll`**，必须从原厂 SDK 获取，非改代码可解决。

### 4.4 授权/加密狗模块跨平台（待确认）

根文档记载 Linux 侧授权为独立旁路 `PstechEncryption.so`，由 C# App 启动时校验、`DetectionRuntime` 每 600 帧复检。`sdk/visage/lib/windows/` 下存在 `jiamigou.lib`（加密狗）与 `mirrorsdll.lib`，疑似 Windows 侧授权库，但其与 C# 授权校验的集成方式**本次调研未覆盖**。Windows 移植时需单独确认授权模块的跨平台实现与运行时依赖。

---

## 五、改造工作量评估

| 工作项 | 改动位置 | 工作量 | 依赖外部资源 |
|--------|----------|--------|-------------|
| CMakeLists.txt 跨平台分支 | `CMakeLists.txt` | 中 | 否 |
| UsbCamera 条件编译/排除 | `UsbCamera.cpp` | 小 | 否 |
| C# 互操作层 | `NativeMethods.cs` | 无 | 否 |
| C# 应用发布 | `dotnet publish -r win-x64` | 无 | 否 |
| 海康/DVT 运行时 DLL | `sdk/*/lib/windows/` | 取得即可 | 是（原厂 SDK） |
| Visage Windows 运行时 | `sdk/visage/lib/windows/` | 待确认 | 可能是 |
| OpenCV/FFmpeg Windows 库 | 本地环境 | 安装配置 | 是（第三方） |
| 授权模块跨平台 | 待定 | 待确认 | 可能是 |

**结论：代码改动集中在 2 个文件（`CMakeLists.txt` + `UsbCamera.cpp`），C# 侧零改动；主要成本在外部第三方 Windows 运行时库的获取与配置。**

---

## 六、推荐改造路线

### 路线一：最小化验证（推荐先做）

只编译 `file`（VirtualCamera）+ `pstrack`（Visage）通路，排除 hik/dvt/usb，使用现有的 `libVisageVision64.lib` 验证：

1. `PstechNative.dll` 能否在 MSVC 下成功编译并导出 C ABI 符号
2. C# 端能否通过 P/Invoke 成功加载该 DLL 并调用

优点：不依赖海康/DVT 的 Windows DLL，可在最小外部依赖下打通「C++ 编译 → 导出 → C# 加载」的完整链路，快速验证可行性。

### 路线二：完整移植

在路线一打通后，逐项补齐：
1. 改造 `CMakeLists.txt` 增加完整 Windows 分支（含 hik/dvt/usb）
2. 从原厂获取并放入海康、DVT 的 Windows `.dll`
3. 确认 Visage Windows 运行时依赖（OpenVINO 是否需额外 DLL）
4. 处理 `UsbCamera.cpp` 的 V4L2 → DSHOW/MSMF
5. 确认授权模块（加密狗）Windows 集成
6. 准备 OpenCV/FFmpeg Windows 开发库与运行时分发

### 路线三：仅依赖排查

若暂不动代码，可先专项排查 Visage Windows 版的 OpenVINO/TBB 依赖关系，确认运行时是否还缺 DLL，为后续决策提供依据。

---

## 附录：关键证据索引

| 结论 | 证据位置 |
|------|----------|
| 导出层已跨平台 | `src/native/export/PstechExport.cpp:5-9` |
| C# 库名跨平台 | `src/managed/Pstech.Core/NativeMethods.cs:33` |
| C# 未锁定 RID | `src/managed/Pstech.App/Pstech.App.csproj:8` |
| 仅 UsbCamera 含 Linux 代码 | `src/native/plugins/UsbCamera.cpp:3,14,23,25` |
| CMake Linux 硬编码 | `CMakeLists.txt:8,30-35,41-48` |
| Windows SDK 缺 DLL | `sdk/hikvision/lib/windows/`、`sdk/dvt/lib/windows/`（仅 `.lib` + `README.txt`） |
| Visage Windows 库齐全 | `sdk/visage/lib/windows/libVisageVision64.lib` 等 |
| 无 Windows 构建脚本 | 全项目未发现 `.bat`/`.sln`/`.vcxproj`/`CMakePresets.json`/`.ps1` |

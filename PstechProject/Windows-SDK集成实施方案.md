# Windows SDK 集成实施方案（海康 + Visage + DVT + USB 全通路）

> 配套文档：[Windows编译可行性评估.md](Windows编译可行性评估.md)
> 制定日期：2026-06-21（覆盖上一版，新增 DVT 与 USB 通路）
> 任务范围：将海康 HCNetSDK、Visage、DVT 三家摄像头 SDK 的 Windows 版集成进项目，并使 Windows 兼容 USB 摄像头；目标输出 `PstechNative.dll`
> 约束：本方案仅做规划与文件落位设计，**暂不改动任何代码**
> 外部 SDK 来源：
> - 海康：`E:\HCNetSDKV6.1.9.48_build20230410_win64_20250312151005`
> - Visage：`E:\visageSDK-Windows-64bit_v9.1.1`
> - DVT（迪威泰）：`E:\迪威泰摄像机sdk_release_2.2.3_20191223\sdk_release`

---

## 一、核心结论

| 通路 | 集成可行性 | 关键点 |
|------|-----------|--------|
| 海康 HCNetSDK V6.1.9.48 | 可直接集成 | `.lib + .dll` 全套齐全；`HikCamera.h` 已预留 Windows 分支 |
| DVT 2.2.3 | 可直接集成 | Windows `.lib + .dll` 已补齐；`DvtCamera.h` 已预留 `#ifdef _WIN32` 回调；版本与项目现有头文件完全一致 |
| USB 摄像头 | 需小幅改代码 | `UsbCamera.cpp` 两处 V4L2/POSIX 改 DSHOW/MSMF |
| Visage v9.1.1 | 可集成，有授权关卡 | 标准动态库需 `initializeLicenseManager()`，项目代码当前无此调用 |

一句话：**四个通路的第三方库均已齐备；阻塞项收敛为「1 个 Visage 授权决策 + 4 处编译/运行代码适配 + 2 个需自备的基础库（OpenCV4 / FFmpeg4）」。**

---

## 二、三家 SDK + USB 盘点

### 2.1 海康 HCNetSDK V6.1.9.48（win64）

| 类别 | 内容 | 落位 |
|------|------|------|
| 头文件 | `HCNetSDK.h`、`DataType.h`、`DecodeCardSdk.h`、`plaympeg4.h` | `sdk/hikvision/include/` |
| 导入库 | `HCNetSDK.lib`、`HCCore.lib`、`PlayCtrl.lib`、`GdiPlus.lib` | `sdk/hikvision/lib/windows/` |
| 运行时(顶层 18 dll) | `HCNetSDK/HCCore/PlayCtrl/GdiPlus/SuperRender/AudioRender/hlog/hpr/zlib1/libcrypto-1_1-x64/libssl-1_1-x64/libmmd/NPQos/OpenAL32/HXVA/HmMerge/MP_Render/YUVProcess.dll` | 部署目录 |
| 运行时组件目录 | `HCNetSDKCom/`（`HCCore/HCPreview/HCAlarm/StreamTransClient/SystemTransform/libiconv2` 等） | 部署目录，**整目录原样复制，名称不可改** |

### 2.2 DVT 2.2.3（win x64）

| 类别 | 内容 | 落位 |
|------|------|------|
| 头文件 | `nsddefines.h`、`nsdnetinterface.h`、`vaplaysdk.h`（**与项目现有完全一致**） | `sdk/dvt/include/`（现有即可） |
| 导入库 | `lib/win/x64/NsdNetSDK_x64.lib`（项目仅链接此一个；`vaplaysdk64.lib` 不需要） | `sdk/dvt/lib/windows/` |
| 运行时(11 dll) | `NsdNetSDK_x64.dll`、`PanoRenderOGL.dll`、`vaplaysdk64.dll`、`hdxdraw64.dll`、`glew32.dll`、`avcodec-56.dll`、`avutil-54.dll`、`opencv_core2411.dll`、`opencv_highgui2411.dll`、`msvcp120.dll`、`msvcr120.dll` | 部署目录（`bin/win/x64` 整套复制） |

> DVT 自带的 `avcodec-56.dll`/`avutil-54.dll`（FFmpeg 2.6）与 `opencv_*2411.dll`（OpenCV 2.4.11）**仅供 `NsdNetSDK_x64.dll` 内部使用**，详见 §三的 FFmpeg/OpenCV 说明。

### 2.3 Visage v9.1.1（64bit）

| 类别 | 内容 | 落位 |
|------|------|------|
| 头文件 | `include/` 全套 35 个 | `sdk/visage/include/` |
| 导入库 | `libVisageVision64.lib`、`libVisageAnalyser64.lib`、`libVisageGaze64.lib` | `sdk/visage/lib/windows/` |
| 运行时核心 | `libVisageVision64/Analyser64/Gaze64.dll` | 部署目录 |
| 运行时 OpenVINO | `inference_engine*.dll`(4)、`MKLDNNPlugin.dll`、`OVPlugin.dll`、`ngraph.dll` | 部署目录 |
| 运行时其他 | `tbb.dll`、`tbbmalloc.dll`、`opencv_*2411.dll`、`msvcp120/msvcr120.dll` | 部署目录 |
| 授权工具 | `registrationmanager.exe`（申请 license） | 开发机使用，不入库 |
| 模型数据 | 项目现有 `sdk/visage/data/`（`.cfg`+`vfa/vfr/vft`+`plugins.xml`）已具备 | 部署目录 `data/` |

### 2.4 USB 摄像头

无第三方 SDK，走 OpenCV 内置后端。Windows 用 `cv::CAP_DSHOW`（DirectShow）或 `cv::CAP_MSMF`（Media Foundation）替代 Linux 的 `cv::CAP_V4L2`，详见 §五.4。

---

## 三、版本与兼容性核查结论

| 核查项 | 结论 | 风险 |
|--------|------|------|
| DVT 头文件版本 | 新 SDK include 与项目现有 `sdk/dvt/include/` **完全一致**（同 2.2.3） | 无 |
| DVT 回调跨平台 | `DvtCamera.h:28-32` 已写 `#ifdef _WIN32 → WINAPI/HANDLE`，并 `#undef BOOL` 防宏冲突 | 无 |
| 海康头文件 | 新 SDK 为 V6.1.9.48；代码用 `NET_DVR_Login_V40` 等 V40 稳定接口 | 低（需编译验证结构体） |
| 海康 `PlayM4.h` | 新 SDK 未提供，需**保留项目现有** `sdk/hikvision/include/PlayM4.h` | 低 |
| Visage 头文件/API | 与项目现有一致，`VisageTracker/FaceData/getFP` 等全兼容 | 低 |
| Visage 链接模式 | 新版为**动态库**（导入库+dll），现 Linux 为静态库+`-DVISAGE_STATIC` | 中（Windows 须去该宏，§五.1） |
| Visage 授权 | 标准版需 `initializeLicenseManager()`，项目代码无此调用 | **高（§六）** |
| 项目主体 OpenCV | **OpenCV 4**（`OpenCV_DIR=.../cmake/opencv4`），Windows 需自备 OpenCV4 | 中（§七.1） |
| 项目 FFmpeg API | `FfmpegDecoder.h` 用 FFmpeg **4.x** API（`avcodec_send_packet`/`avcodec_receive_frame`/`const AVCodec*`/`av_packet_alloc`），且需 **swscale** | 中（§七.2） |
| DVT 自带 FFmpeg | 仅 `avcodec-56`/`avutil-54`（FFmpeg **2.6**，2015），**无 swscale**，API 不兼容项目代码 | 中——**不可用于项目主体**，仅供 DVT 内部 |
| 第三方自带 OpenCV2411 与主体 OpenCV4 | dll 名不同（`opencv_core2411` vs `opencv_world4xx`），各 dll 内部隔离，可共存 | 低 |
| VC++ 运行时 | DVT/Visage 为 VS2013（`msvcp120/msvcr120`）；项目主体由 MSVC(VS2017+) 编译，另需 VC++ 2015-2022 运行时 | 低（两套运行时可共存） |

---

## 四、文件落位清单（不改代码，可立即执行）

> 操作前先备份项目现有 `sdk/hikvision/lib/windows/`、`sdk/visage/lib/windows/`、`sdk/dvt/lib/windows/`（含旧 `.lib`，作回退）。

### 4.1 海康 → `sdk/hikvision/`

| 源 | 目标 | 操作 |
|----|------|------|
| `头文件/HCNetSDK.h`、`DataType.h`、`DecodeCardSdk.h`、`plaympeg4.h` | `sdk/hikvision/include/` | 覆盖 |
| （保留现有）`PlayM4.h`、`LinuxPlayM4.h` | `sdk/hikvision/include/` | **保留** |
| `库文件/HCNetSDK.lib`、`HCCore.lib`、`PlayCtrl.lib`、`GdiPlus.lib` | `sdk/hikvision/lib/windows/` | 覆盖（现有为 2018 旧版） |

### 4.2 DVT → `sdk/dvt/`

| 源 | 目标 | 操作 |
|----|------|------|
| `include/nsddefines.h`、`nsdnetinterface.h`、`vaplaysdk.h` | `sdk/dvt/include/` | 与现有一致，可保留或覆盖 |
| `lib/win/x64/NsdNetSDK_x64.lib` | `sdk/dvt/lib/windows/NsdNetSDK_x64.lib` | 覆盖（确保与新 dll 配套） |

### 4.3 Visage → `sdk/visage/`（已定路线 B：沿用 Pstech 定制库）

| 项目 | 处理 | 说明 |
|------|------|------|
| `sdk/visage/include/` | **保持现有不动** | 已与项目代码匹配，无需用新 SDK 覆盖 |
| `sdk/visage/lib/windows/libPstechVision64.lib`、`libPstechGaze64.lib`、`libPstechAnalyser64.lib`、`jiamigou.lib`、`mirrorsdll.lib` | **保留沿用** | 定制导入库，链接对象 |
| Pstech 定制 Windows 运行时 dll（`libPstechVision64.dll` 等 + 其配套依赖） | **向供应商索取** | 当前缺失，是路线 B 唯一外部依赖（详见 §六） |

> 用户提供的新 Visage v9.1.1 SDK 在路线 B 下**不直接作为链接对象**，但其 `bin64/` 可作为索取定制 dll 时的「依赖版本参照」（OpenVINO/TBB/opencv2411 版本比对）。

### 4.4 运行时 DLL → 部署目录（建议 `output/bin/windows/managed/`，与 `PstechNative.dll` 同级）

| 来源 | 文件 |
|------|------|
| 海康 | §2.1 顶层 18 dll + `HCNetSDKCom/` 整个文件夹 |
| DVT | §2.2 的 `bin/win/x64` 全部 11 dll |
| Visage（路线 B） | 供应商提供的 `libPstechVision64.dll` 等定制 dll **及其配套依赖**（OpenVINO `inference_engine*`/`ngraph`/`MKLDNNPlugin`/`OVPlugin`、`tbb`/`tbbmalloc`、`opencv_*2411`）。优先用供应商整套；缺失项可从 v9.1.1 `bin64/` 补（须版本匹配） |
| 共享去重 | `msvcp120.dll`/`msvcr120.dll`、`opencv_*2411.dll` 在 DVT/Visage 间重复，保留**一份**即可 |
| 自备 | OpenCV4 运行时 dll（§七.1）、FFmpeg4 运行时 dll（§七.2）、VC++2015-2022 运行时（`vcruntime140.dll` 等） |
| Visage 模型 | `sdk/visage/data/` → 部署目录 `data/` |

---

## 五、关联代码改动预告（编译/运行必需，本次不做）

### 5.0 已为 Windows 预留、无需改动的部分

| 位置 | 已预留 |
|------|--------|
| `PstechExport.cpp:5-9` | `#ifdef _WIN32 __declspec(dllexport)` |
| `HikCamera.h:4-8` | `#ifdef __linux__ LinuxPlayM4.h #else PlayM4.h` |
| `DvtCamera.h:3-5,28-32` | `#undef BOOL` + `#ifdef _WIN32 WINAPI/HANDLE` 回调 |
| `NativeMethods.cs:33` | `DllName="PstechNative"` 跨平台库名 |
| `Pstech.App.csproj` | 未锁定 `RuntimeIdentifier` |

### 5.1 CMakeLists.txt 增加 Windows 分支（构建系统）

- 链接 §4.1/§4.2/§4.3 的 Windows `.lib`（替代 `.so`/`.a`）
- **关键：Windows 分支去掉 `-DVISAGE_STATIC`**（现第 8 行）。新 Visage 为动态库，保留该宏会导致符号修饰与导入库的 `__declspec(dllimport)` 不匹配而链接失败
- 移除 GCC 专有选项（`-fpermissive -fopenmp` 等），改 MSVC 等价
- 平台宏 `-DLINUX` 改为条件定义
- 链接自备的 OpenCV4 + FFmpeg4（§七）

### 5.2 CameraFactory.cpp 弱符号（MSVC 不兼容）

`src/native/plugins/CameraFactory.cpp:12-15` 的 `__attribute__((weak))` MSVC 不支持。需改为按编译进来的 camera 插件做 `#ifdef` 条件提供默认实现，或用 MSVC `/alternatename` 链接器方案。

### 5.3 PsTrackWrapper.cpp POSIX 调用

`src/native/pstrack/PsTrackWrapper.cpp:25-26` 的 `PATH_MAX`/`realpath` 在 MSVC 无。Windows 等价：`char absPath[_MAX_PATH]; _fullpath(absPath, cfgPath, _MAX_PATH);`，加 `#ifdef _WIN32` 分支。

### 5.4 UsbCamera.cpp 适配 Windows（本次新增需求）

`src/native/plugins/UsbCamera.cpp` 两处需改：

| 行 | Linux 现状 | Windows 改造 |
|----|-----------|-------------|
| 3 | `#include <unistd.h>` | `#ifdef _WIN32` 移除 |
| 13-17 | `CheckDeviceExist` 用 `/dev/video{N}` + `stat()` 探测 | Windows 改为直接尝试 `cv::VideoCapture(idx, CAP_DSHOW)` 打开判断，或用 DShow 设备枚举 |
| 25 | `m_cap.open(m_idx, cv::CAP_V4L2)` | `#ifdef _WIN32 cv::CAP_DSHOW`（或 `CAP_MSMF`）`#else cv::CAP_V4L2` |

> 其余逻辑（`std::async` 超时打开、Mock 兜底、`CaptureLoop`、MJPG fourcc）均跨平台，无需改。

---

## 六、Visage 授权方案（已定：路线 B — 沿用 Pstech 定制库）

**决策**：沿用项目现有 Pstech 定制 `.lib`，向供应商索取配套定制 Windows dll。**`PsTrackWrapper.cpp` 授权部分不改**（保持与 Linux 一致，无 `initializeLicenseManager()`）。

实施要点：

1. **链接对象**：`sdk/visage/lib/windows/libPstechVision64.lib`（+ `libPstechGaze64.lib`、`libPstechAnalyser64.lib`、`jiamigou.lib`、`mirrorsdll.lib`），保持现有，不用 v9.1.1 标准库替换
2. **头文件**：`sdk/visage/include/` 保持现有不动
3. **向供应商（Pstech/Visage）索取**：定制版 `libPstechVision64.dll` 等运行时 dll，**且必须连同其配套依赖一起索取**（OpenVINO `inference_engine*`/`ngraph`/`MKLDNNPlugin`/`OVPlugin`、`tbb`/`tbbmalloc`、`opencv_*2411`、`msvcp120`/`msvcr120`）——这是路线 B 唯一的外部阻塞项

> 风险提示：定制 dll（2022 年版）所依赖的 OpenVINO/TBB 版本，须与随附运行时一致。**不要**用 v9.1.1 `bin64/` 的运行时去拼凑 2022 定制 dll，除非确认版本兼容；最稳妥是要求供应商提供「定制 dll + 其原配运行时」整套。
>
> 索取时建议向供应商索要：定制库对应的 **Visage SDK 基线版本号** + **完整 Windows 运行时清单**，便于核对。

---

## 七、需自备的两个基础库：OpenCV4 / FFmpeg4（Windows）

这两个是**项目主体**依赖（非摄像头 SDK）。Linux 端由系统包提供（`opencv4` + 系统 FFmpeg），Windows 需自备预编译开发包。建议统一放进项目 `sdk/` 下，与现有第三方 SDK 组织一致、自包含、随项目走。

### 7.1 OpenCV 4.x

| 项 | 内容 |
|----|------|
| 官方下载 | <https://opencv.org/releases/> 或 <https://github.com/opencv/opencv/releases>（自解压 exe，已含 VS 预编译 lib/dll） |
| 推荐版本 | **OpenCV 4.10.0**（稳定、vc16 通用于 VS2019/2022）；4.8.0–4.12.0 任选，**避开 5.0**（大版本 API 变动，项目代码为 OpenCV4 API） |
| 包文件 | `opencv-4.10.0-windows.exe` → 自解压，内部 `build/` 目录 |
| 取用目录 | `build/include/`（头文件）、`build/x64/vc16/lib/opencv_world4100.lib`（链接）、`build/x64/vc16/bin/opencv_world4100.dll`（运行时） |
| 放置位置（推荐 A·自包含） | `sdk/opencv/windows/{include, x64/vc16/lib, x64/vc16/bin}` |
| 放置位置（备选 B·复用 find_package） | 解压到 `C:\opencv`，设环境变量 `OpenCV_DIR=C:\opencv\build\x64\vc16`，现有 `find_package(OpenCV)` 直接生效 |
| 运行时 dll | `opencv_world4100.dll`（release 单一库；调试另需 `opencv_world4100d.dll`，部署用 release） |

> 与各 SDK 自带的 `opencv_*2411.dll`（OpenCV 2.4.11）dll 名不同，可共存——它们供 DVT/Visage 内部用，项目主体用 `opencv_world4xxx.dll`。

### 7.2 FFmpeg 4.x（DVT 通路依赖，必须 shared/dev 包）

| 项 | 内容 |
|----|------|
| 推荐下载 | BtbN <https://github.com/BtbN/FFmpeg-Builds/releases>（选 **win64-shared**，bin/lib/include 一起）；或 gyan.dev <https://www.gyan.dev/ffmpeg/builds/>（选 **shared**，旧版在 old-releases） |
| 关键 | 必须 **shared** 变体（含 `include/`+`lib/`+`bin/`）；**不要** static/essentials-只含 exe 的包 |
| 推荐版本 | **FFmpeg 4.4.x**（与 Linux 端最接近，`avcodec-58`）；取不到则 6.1 LTS / 7.x 亦兼容（`FfmpegDecoder.h` 用的 `send_packet`/`receive_frame`/`const AVCodec*` 在 4.x–8.x 均支持） |
| 放置位置（推荐·自包含） | `sdk/ffmpeg/windows/{include, lib, bin}` |
| 链接对象 | `lib/avcodec.lib`、`lib/avutil.lib`、`lib/swscale.lib`（`FfmpegDecoder.h` 仅用这三个；不用 avformat） |
| 运行时 dll | 见下表对照（avcodec 运行时还依赖 `avutil`、`swresample`，一并复制） |

**FFmpeg dll 版本号对照**（按所选 major 版识别 bin 内文件名）：

| FFmpeg | avcodec | avutil | swscale | swresample |
|--------|---------|--------|---------|------------|
| 4.4 | `avcodec-58` | `avutil-56` | `swscale-5` | `swresample-3` |
| 6.x | `avcodec-60` | `avutil-58` | `swscale-7` | `swresample-4` |
| 7.x | `avcodec-61` | `avutil-59` | `swscale-8` | `swresample-5` |

> 与 DVT 自带的 `avcodec-56.dll`（FFmpeg 2.6）dll 名不同，可共存：DVT 内部用 56，项目 `FfmpegDecoder` 用自备的 58/60/61，互不干扰。

### 7.3 CMake 指向方式（先不改代码，仅说明后续接法）

- **OpenCV**：放置 A 用 `set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/sdk/opencv/windows/x64/vc16")` 后复用 `find_package(OpenCV REQUIRED)`；放置 B 靠环境变量自动找
- **FFmpeg**：Windows 分支 `include_directories(sdk/ffmpeg/windows/include)` + `target_link_libraries(... sdk/ffmpeg/windows/lib/avcodec.lib sdk/ffmpeg/windows/lib/avutil.lib sdk/ffmpeg/windows/lib/swscale.lib)`（替代现有 Linux 的 `find_library(AVCODEC_LIB ...)`）

---

## 八、建议实施顺序（待决策确认后）

1. **决策**：确认 §六 Visage 授权路线（A / B）
2. **自备基础库**：准备 OpenCV4 + FFmpeg4 的 Windows 开发包
3. **文件落位**（不改代码）：执行 §四 全部复制/替换 + 备份旧库
4. **构建适配**（改代码）：§5.1 CMake Windows 分支 → §5.2 弱符号 → §5.3 PsTrackWrapper POSIX → §5.4 UsbCamera DSHOW
5. **授权适配**（仅路线 A）：接入 license + `initializeLicenseManager()`
6. **编译**：MSVC x64 生成 `PstechNative.dll`
7. **运行时组装**：§4.4 全部 dll + `HCNetSDKCom/` + Visage `data/` 复制到部署目录
8. **验证**：C# 加载 `PstechNative.dll` → file/usb/hik/dvt 四通路取帧 → Visage 追踪出点

---

## 九、待确认事项汇总

| 编号 | 事项 | 状态 |
|------|------|------|
| Q1 | Visage 授权路线 | **已定：路线 B**（沿用定制库，向供应商索取定制 dll，§六） |
| Q2 | OpenCV4 / FFmpeg4 放置位置 | **已完成：OpenCV 4.10.0 + FFmpeg 4.4 已下载就位**于 `sdk/opencv/windows/`、`sdk/ffmpeg/windows/`（含 README + CMake 接入说明） |
| Q3 | Visage 运行时 dll | **已完成**：用户提供 `libVisageVision64.dll`（自包含），符号验证 5/5 匹配，已就位（§十.4） |
| Q4 | 海康新版 `HCNetSDK.h`(V6.1.9.48) 与现有代码结构体兼容性 | 低（编译验证） |
| Q5 | 运行时 dll 输出目录约定（建议 `output/bin/windows/managed/`） | 低 |

---

## 十、编译验证结果（2026-06-21，已完成）

**环境**：Visual Studio Community 2022（17.14, MSVC 14.44）+ 自带 CMake 3.31
**结果**：`PstechNative.dll`（79.5 KB）编译链接成功，**错误 0**。构建目录 `build_win/Release/`。

### 10.1 编译实战中的额外修复（方案 §五 4 处之外新发现）

| 文件 | 问题 | 修复 |
|------|------|------|
| `src/native/plugins/DvtCamera.h` | DVT 头文件 Windows 分支依赖 `windows.h` 提供 `HANDLE/WPARAM/WINAPI`，但未引入 | Windows 下 `#include <windows.h>`（+`NOMINMAX`） |
| `sdk/hikvision/include/WindowsPlayM4.h` | 项目 `PlayM4.h` 分发器需要此文件但缺失；且海康原文件 guard 与分发器冲突、`PLAYM4_API` 的 `"C"__declspec` 在 C++11 触发 user-defined-literal 错误、`_WINDLL` 误判方向 | 从海康 Demo `PlayM4.h` 复制；guard 改 `_WINDOWS_PLAYM4_H_`；`PLAYM4_API` 统一为 `dllimport` 并补空格 |
| `src/native/plugins/HikCamera.h/.cpp` | Win64 下 `LONG=long≠int`：`PlayM4_GetPort` 参数、`DecCallBack` 回调签名不匹配；`nUser(long)` 无法承载 64 位 `this` 指针 | `m_nPort`→`LONG`；`DecCallBack` 按平台分支签名；回调改用文件内静态实例指针 |
| `sdk/opencv/windows/x64/vc16/lib/` | OpenCV CMake 默认校验已精简删除的 debug dll | 移除 `OpenCVModules-debug.cmake` + debug lib，纯 Release |

### 10.2 导出符号验证（C ABI，与 C# P/Invoke 契约一致）

10 个 `Pstech_*` 函数全部正确导出：`Pstech_Alg_Init`/`Track`、`Pstech_Camera_Init`/`Start`/`Stop`/`LockAndGet`/`Unlock`/`GetBrightness`/`SetBlackScreenConfig`、`Pstech_SetStatusCallback`。

### 10.3 运行时依赖 DLL 清单（dumpbin /DEPENDENTS）

| DLL | 来源 | 状态 |
|-----|------|------|
| `HCNetSDK.dll` / `PlayCtrl.dll` | 海康 SDK | 已提供 |
| `NsdNetSDK_x64.dll` | DVT SDK | 已提供 |
| `libVisageVision64.dll` | Visage（Pstech 定制打包，自包含无 OpenVINO 依赖） | **已就位**：用户提供，符号验证 5/5 匹配；链接对象同步改为标准版 `libVisageVision64.lib`(v9.1.1) |
| `opencv_world4100.dll` | `sdk/opencv/windows` | 已就位 |
| `avcodec-58`/`avutil-56`/`swscale-5.dll` | `sdk/ffmpeg/windows` | 已就位 |
| `MSVCP140`/`VCRUNTIME140*.dll` | VC++ 2015-2022 运行时 | 部署机装 `vc_redist.x64.exe` |
| `api-ms-win-crt-*` / `KERNEL32.dll` | Windows 系统 | 系统自带 |

**结论**：C++ 原生库 Windows 编译链路**完全打通**，所有运行时第三方库依赖已就位（含用户提供的 `libVisageVision64.dll`，符号验证 5/5 匹配）。

### 10.4 Visage dll 替换（2026-06-21 追加）

用户提供 `libVisageVision64.dll`（基线 v9.1.1，经 Pstech 定制打包）。处理：

1. CMake 链接对象由定制 `libPstech*64.lib` 改为标准 `libVisage*64.lib`（复制 v9.1.1 的 lib 到 `sdk/visage/lib/windows/`），使 `PstechNative.dll` 依赖名为 `libVisageVision64.dll`，自洽
2. 重新编译成功（错误 0），`dumpbin /IMPORTS` 确认依赖变更
3. **符号验证**：`PstechNative.dll` 实际仅需 5 个 Visage 符号（`FaceData` 构造、`VisageTracker` 构造/析构/`track`、`FDP::getFP`），用户 dll 导出 767 个符号全部覆盖，缺失 0
4. dll 已复制到 `build_win/Release/`（运行）与 `sdk/visage/lib/windows/`（持久）

> 授权说明：`PsTrackWrapper.cpp` 维持无 `initializeLicenseManager()` 不变。实测授权有效（§十.5）。
>
> **重要修正**：用户 dll 的**静态导入表**不含 OpenVINO（`dumpbin /DEPENDENTS` 看不到），但 Visage 神经网络引擎（ViNNIE）在**运行时动态 `LoadLibrary` `OVPlugin.dll`**（及 `inference_engine*`/`MKLDNNPlugin`/`ngraph`/`tbb`）来跑 `.vino` 模型。**部署仍须随附 OpenVINO 全套运行时**——「自包含、部署简化」的结论不成立。

### 10.5 file 通路 + Visage 追踪实测（2026-06-21，通过）

用 Python ctypes 直接驱动 `PstechNative.dll` 的 C ABI（绕过 C# App 授权），读 `test.mp4`（file 通路）逐帧 Visage 追踪，**15 帧全部成功，10 帧检出人脸**：

| 指标 | 结果 |
|------|------|
| dll 加载 | `PstechNative.dll` + 42 个依赖 dll 全部解析成功 |
| 视频读取 | 15/15 帧 FRAME_OK（VirtualCamera + OpenCV ffmpeg 后端读 mp4） |
| Visage 追踪 | 首帧 265ms（加载模型），后续帧 2-10ms；10 帧检出人脸 |
| 人脸数据 | 眼睛闭合度（含 `eye=0.00` 闭眼帧）、头部姿态 [pitch,yaw,roll]、238 个关键点 |
| 授权 | 输出有效人脸数据，**用户 dll 授权有效**（非水印/拒绝） |

实测中解决的三个运行时问题：
1. **OpenVINO 运行时缺失**：ViNNIE 动态加载 `OVPlugin.dll` 失败 → 补齐 v9.1.1 OpenVINO 全套
2. **模型路径**：Visage 数据根 = `PstechNative.dll` 所在目录（非 cwd），cfg 相对路径 `vft/...` 须相对 dll 目录 → 模型须与 dll 同级（部署时 data 内容平铺到 dll 目录，或确保 dll 在 data 根）
3. **多帧崩溃**：首帧 track 长耗时（265ms）期间 RingBuffer 被采集线程覆盖 → 调用方每帧拷贝图像后再 track（C# 端 `DetectionRuntime` 用 `Image.LoadPixelData` 拷贝，机理相同，故真实 App 无此问题）

**结论**：海康 + DVT + Visage 的 Windows 集成在 file 通路 + Visage 人脸追踪上**端到端验证通过**。

### 10.6 海康网络摄像头实测（2026-06-23，通过）

用真实海康摄像头（`192.168.1.69:8000`, admin）测 hik 通路，端到端成功：

| 环节 | 结果 |
|------|------|
| 网络 | ping 1-2ms，8000 端口 `TcpTestSucceeded: True` |
| SDK 登录 | `NET_DVR_Login_V40` 成功，`Camera_Init` 返回 0 |
| 取流 | 海康输出 1280x720 真实码流，`ICamera::PushFrame` 缩放到 640x480 |
| PlayM4 解码 | 50/50 帧 FRAME_OK——之前修复的 `m_nPort` LONG 化、`DecCallBack` 平台分支签名、静态实例指针、`WindowsPlayM4.h` dllimport 全部正确工作 |
| 画面亮度 | 107.8（真实画面，非黑屏 Mock 的 state=2） |
| Visage 追踪 | 正常运行，faces=0（监控画面中无人脸，非算法问题；file 通路已证明检测人脸正常） |

**结论**：海康网络摄像头通路在 Windows 上**登录 + 取流 + PlayM4 解码 + Visage 追踪端到端打通**。至此 **file + hik 两条主要通路**均在 Windows 验证通过。

---

## 附录：关键证据索引

| 结论 | 证据位置 |
|------|----------|
| 海康 DLL 全套齐全 | 海康 `库文件/*.dll` + `HCNetSDKCom/*.dll` |
| 海康组件目录加载要求 | `HCNetSDKCom文件夹必须加载到工程.txt` |
| DVT Windows dll 齐全 | DVT `bin/win/x64/`（11 dll，含 `NsdNetSDK_x64.dll`） |
| DVT 版本与项目一致 | DVT `include/` == `sdk/dvt/include/`（nsddefines/nsdnetinterface/vaplaysdk.h） |
| DvtCamera 已预留 Windows | `src/native/plugins/DvtCamera.h:3-5,28-32` |
| 项目主体 OpenCV4 | `build_linux/CMakeCache.txt` → `OpenCV_DIR=.../cmake/opencv4` |
| 项目 FFmpeg4 API + swscale | `src/native/plugins/FfmpegDecoder.h:3-5,16-44` |
| DVT 自带 FFmpeg 仅 2.6 | DVT `bin/win/x64/avcodec-56.dll`、`avutil-54.dll`（无 swscale） |
| Visage 动态库 | Visage `lib/*.lib`(导入库) + `bin64/*.dll` |
| Visage 需 license 初始化 | `Samples/OpenGL/source/common/FolderManager.cpp:113` |
| VISAGE_STATIC 控制链接修饰 | `sdk/visage/include/Base.h:29-36` |
| PsTrackWrapper POSIX 依赖 | `src/native/pstrack/PsTrackWrapper.cpp:25-26` |
| CameraFactory 弱符号 | `src/native/plugins/CameraFactory.cpp:12-15` |
| UsbCamera V4L2/POSIX | `src/native/plugins/UsbCamera.cpp:3,13-17,25` |

# PstechFatigueSystem -- 威视疲劳检测系统

> 项目根级总览文档。各子模块的详细文档见 [模块导航](#四模块导航) 中的链接。

> 变更记录 (Changelog)
>
> | 时间 | 操作 | 说明 |
> |------|------|------|
> | 2026-06-16 | 新建 | 初始化项目根级总览文档（架构、数据流、互操作契约、构建运行、开发须知） |

---

## 一、项目简介

威视疲劳检测系统是一款基于计算机视觉的实时监控软件，通过摄像头采集画面，对操作人员的**疲劳（闭眼）、分心（头部偏转）、离岗（无人）、手机使用、摄像头异常（黑屏）**进行智能识别与多级报警。

- **应用场景**：安检、监控值守等需要持续关注人员状态的岗位
- **运行平台**：Linux（Ubuntu），需硬件加密狗授权
- **交互方式**：浏览器访问 `http://设备IP:80`（Web 管理）+ `http://设备IP:8080`（实时视频流）
- **面向用户的文档**：[疲劳检测系统功能介绍.md](疲劳检测系统功能介绍.md)、安装新版威视疲劳检测步骤.docx

---

## 二、技术栈

| 层 | 技术 | 说明 |
|----|------|------|
| 原生层 | C++17 / CMake 3.10+ / OpenCV / FFmpeg / OpenMP | 摄像头采集、人脸追踪封装，编译为 `libPstechNative.so` |
| 业务层 | C# / .NET 8 (AOT) | 检测管线、报警、Web 服务、配置热加载、手机检测 |
| 前端 | React 19 / Ant Design 5 / Vite 7 / Axios | Web 配置管理 SPA |
| 推理 | ONNX Runtime 1.18 | 手机检测（YOLO + MobileNetV2 分类器） |
| 人脸算法 | Visage SDK（静态库） | 人脸检测、关键点、头部姿态、眼睛闭合度 |
| 设备 SDK | 海康威视 SDK / DVT SDK | 网络摄像头接入 |
| 报警硬件 | System.IO.Ports（串口 9600/8N1） | 声光报警器 |

---

## 三、整体架构

三层混合架构，C# 通过 **P/Invoke** 调用 C++ 原生库；前端通过 **REST API** 与 C# 后端通信。

```
┌──────────────────────────────────────────────────────────────┐
│  前端 SPA  (Pstech.Web, React 19 + Ant Design)                 │
│  vite build → wwwroot/                                          │
└───────────────┬──────────────────────────┬─────────────────────┘
        REST /api/* (端口 80)        MJPEG 视频流 (端口 8080)
                │                            │
┌───────────────▼────────────────────────────▼───────────────────┐
│  C# 应用层  (.NET 8 AOT)                                          │
│                                                                  │
│  Pstech.App   ── 入口：授权检查 + 配置加载 + 三服务编排           │
│      │                                                           │
│      ├── WebServer(80) + ApiRouter + 5 Handlers ── 配置管理      │
│      ├── MjpegServer(8080) ───────────────────── 视频推流        │
│      └── DetectionRuntime ─── 检测主循环(~30fps)                 │
│              └── PipelineAnalyzer ── 黑屏→YOLO→手机→ROI→         │
│                     PsTrack→离岗→疲劳→分心 → MonitorState        │
│                        ├── AlarmManager ── HTTP 上报 + 截图       │
│                        ├── FrameDrawer ─── 画面叠加              │
│                        └── PhoneDetection ─ ONNX 推理            │
│  Pstech.Core  ── P/Invoke 互操作 (NativeFaceData + Native)       │
└───────────────────────────┬─────────────────────────────────────┘
                   P/Invoke (Cdecl) ↕  零拷贝帧指针 (IntPtr)
┌───────────────────────────▼─────────────────────────────────────┐
│  C++ 原生层  (libPstechNative.so)                                 │
│                                                                  │
│  PstechExport.cpp ── C ABI 导出 (Pstech_Camera_* / Pstech_Alg_*) │
│      ├── CameraFactory → ICamera 实现                            │
│      │      ├── VirtualCamera(file) / UsbCamera(usb)             │
│      │      └── HikCamera(hik) / DvtCamera(dvt)                  │
│      │              └── RingBuffer（线程安全帧缓冲）             │
│      └── PsTrackWrapper ── 封装 Visage SDK 人脸追踪              │
└───────────────────────────┬─────────────────────────────────────┘
                            │ 链接
┌───────────────────────────▼─────────────────────────────────────┐
│  第三方 SDK  (sdk/)   hikvision · dvt · visage(libVisageVision.a)│
└──────────────────────────────────────────────────────────────────┘

  独立旁路：PstechEncryption.so ── 硬件加密狗授权（启动校验 + 运行期复检）
```

### 生命周期与服务隔离

`Pstech.App` 启动三个**独立生命周期**的服务，检测启动失败不会阻塞 Web 管理服务（可通过 API 重试）：

| 服务 | 端口 | 权限 | 职责 |
|------|------|------|------|
| WebServer | 80（可配） | root | REST API + 静态前端托管（SPA fallback） |
| MjpegServer | 8080 | root | MJPEG 实时视频流 |
| DetectionRuntime | -- | -- | 后台检测线程（摄像头 + 算法 + 报警） |

---

## 四、模块导航

| 模块 | 路径 | 语言 | 职责 | 文档 |
|------|------|------|------|------|
| 原生层 | `src/native/` | C++ | 摄像头采集 + 人脸追踪封装 + C ABI 导出 | 见下方 [§4.1](#41-原生层-srcnative无独立-claudemd) |
| Pstech.Core | `src/managed/Pstech.Core/` | C# | P/Invoke 互操作（`NativeFaceData` + `Native`） | [CLAUDE.md](src/managed/Pstech.Core/CLAUDE.md) |
| Pstech.Logic | `src/managed/Pstech.Logic/` | C# | 核心业务逻辑（管线/报警/Web/配置/手机检测） | [CLAUDE.md](src/managed/Pstech.Logic/CLAUDE.md) |
| Pstech.App | `src/managed/Pstech.App/` | C# | 应用入口（授权 + 三服务编排） | [CLAUDE.md](src/managed/Pstech.App/CLAUDE.md) |
| Pstech.Web | `src/managed/Pstech.Web/` | React | Web 配置管理前端 | [CLAUDE.md](src/managed/Pstech.Web/CLAUDE.md) |

### 4.1 原生层 (src/native/，无独立 CLAUDE.md)

```
src/native/
├── common/
│   ├── ICamera.h / ICamera.cpp   摄像头抽象基类（帧缩放、黑屏检测、RingBuffer 推送）
│   ├── RingBuffer.h              线程安全环形帧缓冲（GetLatest / Write）
│   ├── PstechStatus.h            帧状态枚举（FRAME_EMPTY/OK/BLACK_SCREEN）+ StatusCallback
│   └── LinuxCompat.h             Linux 兼容定义
├── plugins/
│   ├── CameraFactory.cpp/.h      按 CameraType 创建摄像头实例
│   ├── VirtualCamera.cpp/.h      本地视频文件（file）
│   ├── UsbCamera.cpp/.h          USB 摄像头（usb）
│   ├── HikCamera.cpp/.h          海康网络摄像头（hik）
│   ├── DvtCamera.cpp/.h          DVT 网络摄像头（dvt）
│   └── FfmpegDecoder.h           FFmpeg 解码（DVT 用）
├── pstrack/
│   └── PsTrackWrapper.cpp/.h     封装 Visage SDK，输出 PsTrackFaceData
├── export/
│   └── PstechExport.cpp          C ABI 导出层（C# 入口）
└── tests/                        独立测试可执行（test_hik/dvt/usb/file/pstrack）
```

- **摄像头类型枚举**：`CAM_VIRTUAL_FILE=0`, `CAM_USB=1`, `CAM_HIKVISION=2`, `CAM_DVT=3`
- **帧状态枚举**：`FRAME_EMPTY=0`（无新帧）, `FRAME_OK=1`（正常）, `FRAME_BLACK_SCREEN=2`（黑屏）
- 所有摄像头帧统一在 `ICamera::PushFrame` 中缩放到目标分辨率（`cv::resize`）并做亮度黑屏判定，再写入 RingBuffer

---

## 五、核心数据流（每帧 ~33ms）

1. **采集**（C++ 后台线程）：摄像头插件解码 → `ICamera::PushFrame` 缩放 + 黑屏亮度判定 → `RingBuffer.Write`
2. **取帧**（C# 检测线程）：`Native.Pstech_Camera_LockAndGet(out ptr, out w, out h, out ts)` 返回帧指针 + frameState
3. **追踪**：`Native.Pstech_Alg_Track(ptr, w, h, ref NativeFaceData)` → Visage 输出人脸框/关键点/头部姿态/眼睛闭合度
4. **管线分析**（`PipelineAnalyzer.Update`）：黑屏 → YOLO（间隔帧）→ 手机检测 → ROI → PsTrack → 离岗 → 疲劳 → 分心，产出 `MonitorState`
5. **报警**（`AlarmManager`）：按持续时间/阈值比率判定 Level1/2/3 → HTTP POST（JSON + Base64 截图）+ 串口声光 + 截图落盘
6. **呈现**：`FrameDrawer` 叠加可视化 → JPEG 编码 → `MjpegServer.UpdateFrame` 推流（8080）

报警升级：`持续时间 / 阈值 ≥ Level2Ratio(1.5)` 升 Level2，`≥ Level3Ratio(2.0)` 升 Level3。

---

## 六、C++/C# 互操作契约（最关键的边界）

定义于 C++ `src/native/export/PstechExport.cpp`（导出）与 C# `src/managed/Pstech.Core/NativeMethods.cs`（声明），均为 `CallingConvention.Cdecl`、字符串 `CharSet.Ansi`。

| C ABI 导出函数 | C# 声明 | 说明 |
|----------------|---------|------|
| `Pstech_SetStatusCallback(cb)` | `void` | 设置状态回调 |
| `Pstech_Camera_Init(type, conn, u, p, idx, w, h)` | `int` | 创建并初始化摄像头（0=成功） |
| `Pstech_Camera_SetBlackScreenConfig(checkBlack, threshold)` | -- | 配置黑屏检测 |
| `Pstech_Camera_GetBrightness()` | `double` | 当前画面亮度 |
| `Pstech_Camera_Start()` / `Pstech_Camera_Stop()` | `int` / `void` | 启停采集 |
| `Pstech_Camera_LockAndGet(ptr, w, h, ts)` | `int` | 获取最新帧（返回 frameState：0/1/2） |
| `Pstech_Alg_Init(cfgPath)` | `void` | 初始化 PsTrack 算法 |
| `Pstech_Alg_Track(imgPtr, w, h, ref NativeFaceData)` | `void` | 执行人脸追踪，结果写回结构体 |

### 数据结构 `NativeFaceData`（`[StructLayout(LayoutKind.Sequential)]`）

| 字段 | 类型 | 说明 |
|------|------|------|
| `IsDetected` | int | 是否检测到人脸（0/1） |
| `EyeClosure` | float[2] | 左右眼闭合度（0~1，<0.5 视为闭合） |
| `HeadPose` | float[3] | 头部姿态 [pitch, yaw, roll]（弧度） |
| `FaceRect` | float[4] | 人脸矩形 |
| `Origin` | float[2] | 人脸原点（归一化） |
| `LandmarkCount` / `LandmarksPtr` | int / IntPtr | 关键点数量与指针 |

> **红线约束**：`NativeFaceData` 必须用 `NativeFaceData.Create()` 工厂方法初始化（预分配数组字段），否则 P/Invoke 封送时访问 null 数组导致崩溃。详见 [Pstech.Core/CLAUDE.md](src/managed/Pstech.Core/CLAUDE.md)。

---

## 七、构建与运行

### 7.1 构建 C++ 原生库（Linux）

```bash
cd build_linux          # 或新建构建目录
cmake ..
make -j                 # 产出 libPstechNative.so + 5 个测试可执行
```

- 依赖：`OpenCV`、`FFmpeg`(avcodec/avformat/avutil/swscale)、`ZLIB`、`OpenMP`、`Threads`
- 产物：`build_linux/libPstechNative.so`
- 测试目标：`Test_Hik` / `Test_Dvt` / `Test_Usb` / `Test_File` / `Test_PsTrack`（各自带 `TEST_MODE` 编译宏）

### 7.2 构建前端

```bash
cd src/managed/Pstech.Web
pnpm install
pnpm build               # 输出到 ../Pstech.App/wwwroot/
```

### 7.3 构建 C# 应用（AOT）

```bash
cd src/managed/Pstech.App
dotnet publish -c Release -r linux-x64
# 产物含 Pstech.App（原生 AOT 可执行）+ appsettings.json + phone-detection.json + wwwroot/ + data/*.onnx
```

最终部署产物示例见 `output/bin/linux/managed/`（含 `libPstechNative.so`、Visage `data/` 模型、ONNX 模型）。

### 7.4 运行

```bash
# 直接运行（CLI 可覆盖摄像头配置）
./Pstech.App <mode> [connection]
#   mode: file | usb | hik | dvt
#   connection: file→视频路径, usb→设备索引, hik/dvt→设备索引

# 部署脚本（参见功能介绍文档）
sudo ./universal_run.sh cs_dvt
sudo ./install_service.sh           # 注册 systemd 服务 pstech.service
```

启动前会通过 `PstechEncryption` 校验硬件加密狗，未授权直接退出。

---

## 八、配置文件

| 文件 | 位置 | 序列化上下文 | 说明 |
|------|------|-------------|------|
| `appsettings.json` | Pstech.App | `AppSettingsJsonContext`(PascalCase, 持久化) / `ManagementJsonContext`(读取) | 主配置：Camera / Algorithm / Visual / Report / System |
| `phone-detection.json` | Pstech.App | `ManagementJsonContext` | 手机检测插件配置 |
| `ui-config.json` | Pstech.Web/public | -- | 前端 Tab 显隐配置（构建时复制到 wwwroot） |

### appsettings.json 关键段

| 段 | 关键参数 | 默认 |
|----|----------|------|
| Camera | Type / ConnectionString / Width×Height / CheckBlack / BrightnessThreshold | file / - / 640×480 / true / 10.0 |
| Algorithm | FatigueTimeMs / DistractionTimeMs / AbsenceTimeMs / BlackScreenTimeMs | 2000 / 2000 / 5000 / 1000 |
| Algorithm | MaxPitch / MaxYaw / MaxRoll / OdInterval / PsTrackInterval | 20° / 30° / 20° / 25 / 3 |
| Report | Enable / ApiUrl / Level2Ratio / Level3Ratio / CaptureRetentionDays | true / 127.0.0.1:5000 / 1.5 / 2.0 / 30 |
| System | WebPort / SerialPortPath / PowerControlEnabled / DeviceAlarmEnabled | 80 / /dev/ttyUSB0 / false / true |

### 配置热加载

通过 `ConfigCoordinator` 协调，diff 后分流：算法阈值/可视化/报警/系统/手机规则等**可热加载即时生效**；摄像头/PsTrack/WebPort/串口等**需重启检测**。`appsettings.json` 由 `AppSettingsWriter` 字节级合并写入，**保留原文件注释与布局**。详见 [Pstech.Logic/CLAUDE.md](src/managed/Pstech.Logic/CLAUDE.md)。

---

## 九、开发须知（红线与约定）

1. **P/Invoke 边界**：修改 C 导出函数签名时，必须同步更新 `PstechExport.cpp` 和 `NativeMethods.cs` 双方；`NativeFaceData` 字段顺序/类型必须与 C++ 端严格一致。
2. **`NativeFaceData` 初始化**：永远用 `NativeFaceData.Create()`，禁止裸 `new`。
3. **AOT 兼容**：C# 全程 `PublishAot=true`。新增任何 JSON 序列化的 DTO/配置类型，必须在 `ManagementJsonContext`（API，camelCase）或 `AppSettingsJsonContext`（持久化，PascalCase）中注册 `[JsonSerializable]`，否则运行期序列化失败。
4. **配置写入**：`appsettings.json` 走 `AppSettingsWriter`（保留注释/布局），不要直接 `JsonSerializer.Serialize` 覆盖。
5. **前后端契约**：前端用秒/四方向角度，后端用毫秒/三轴角度；转换在 `fatigueApi.js` 兼容适配层，改动接口字段时两侧同步。
6. **命令注入防护**：执行 Shell 优先用 `LinuxCommandExecutor.ExecuteDirect`（无 shell）；网络/IP 参数经 `IsValidIp/IsValidPort` 校验。
7. **电源控制门闸**：关机/重启 API 受 `System.PowerControlEnabled`（默认 false）保护，未开启返回 403。
8. **安全**：禁止硬编码密钥；摄像头密码等敏感配置不入库；ONNX 模型路径校验防遍历。
9. **YOLO 措辞**：`FatigueAnalyzer.cs` 已弃用（被 `PipelineAnalyzer` 替代，保留备用）；旧 YOLOv8 已清理。

---

## 十、目录结构总览

```
PstechProject/
├── CLAUDE.md                       本文档（项目根总览）
├── CMakeLists.txt                  C++ 构建定义（PstechNative + 5 测试）
├── 疲劳检测系统功能介绍.md          面向用户的功能说明
├── 安装新版威视疲劳检测步骤.docx     部署步骤
├── dvt.jpg                         调试预览界面截图
├── build_linux/                    C++ CMake 构建产物（libPstechNative.so）
├── output/bin/linux/managed/       最终部署产物（C# + native + Visage data + ONNX）
├── sdk/                            第三方 SDK（仅 Linux 库）
│   ├── hikvision/                  海康威视 SDK（include + lib/linux）
│   ├── dvt/                        DVT 摄像头 SDK
│   └── visage/                     Visage 人脸追踪 SDK（libVisageVision.a + data 模型）
└── src/
    ├── native/                     C++ 原生层 → libPstechNative.so（见 §4.1）
    └── managed/                    C# .NET 8 解决方案（PstechManaged.slnx）
        ├── Pstech.Core/            P/Invoke 互操作          [CLAUDE.md]
        ├── Pstech.Logic/           核心业务逻辑              [CLAUDE.md]
        ├── Pstech.App/             应用入口                 [CLAUDE.md]
        └── Pstech.Web/             React 前端               [CLAUDE.md]
```

---

## 十一、常见问题 (FAQ)

**Q: 从哪里开始读代码？**
A: 入口 `src/managed/Pstech.App/Program.cs`（三服务编排）→ 检测核心 `Pstech.Logic/Runtime/DetectionRuntime.cs` + `PipelineAnalyzer.cs` → 互操作边界 `Pstech.Core/NativeMethods.cs` ↔ `src/native/export/PstechExport.cpp`。

**Q: 启动报授权错误？**
A: 确认硬件加密狗已插入、`libPstechEncryption.so` 在运行目录。`DetectionRuntime` 每 600 帧复检，连续 2 次失败会停止检测（但 Web 服务继续）。

**Q: 视频流（8080）无画面但 Web（80）正常？**
A: 检测运行时未启动或异常。查 `/api/system/health`，必要时 `/api/device/restart-detection`。

**Q: Web API 改了配置不生效？**
A: 取决于参数类型（热加载 vs 需重启）。检测未运行时所有变更降级为冷保存。详见 Pstech.Logic 文档。

**Q: 手机检测不工作？**
A: 确认 `phone-detection.json` 存在且 `Enabled=true`、`data/od.onnx` 存在。熔断器触发会暂停（日志 `[PhoneYolo] Circuit breaker OPEN`）。

---

## 十二、维护说明

- 本项目采用**分层 CLAUDE.md**：根文档（本文件）负责全局架构与跨模块约定，各子模块文档负责本模块细节。**修改某模块时，请同步更新对应子模块的 CLAUDE.md 变更记录。**
- 注：现有 4 个子模块文档头部的 `[根目录](../CLAUDE.md)` 链接指向 `src/managed/CLAUDE.md`（managed 层聚合索引，当前尚未创建）。如需补全该中间层索引，可在 `src/managed/` 下新建。

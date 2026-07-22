# Pstech 疲劳识别系统 — 项目架构文档

> 生成日期：2026-03-12

---

## 1. 项目概述

**Pstech 疲劳识别系统** 是一套生产级驾驶员疲劳监测系统，通过实时视频流采集、AI 人脸跟踪与目标检测，对驾驶员的 **疲劳、分心、离岗、打电话** 等危险行为进行识别与多级报警。

**适用场景**：智能驾驶安全监测、商用车队管理、疲劳驾驶预警。

---

## 2. 技术栈

| 层级 | 技术 | 说明 |
|------|------|------|
| **原生层 (C++)** | C++17 / CMake 3.10+ | 相机驱动、人脸跟踪、帧缓冲 |
| **应用层 (C#)** | .NET 8.0 | 业务逻辑、AI 推理、Web GUI |
| **AI 推理** | ONNX Runtime 1.18.0 | YOLOv8n 目标检测 |
| **人脸跟踪** | Visage PsTrack | 468 点关键点 + 眼睛闭合度 + 头部姿态 |
| **图像处理** | OpenCV 4.x / SixLabors.ImageSharp 3.1.6 | 视频采集 + 图像绘制 |
| **视频解码** | FFmpeg (libavcodec/libavformat) | H.264 硬解 |
| **相机 SDK** | 海康威视 / DVT(大华) | IP 网络摄像机接入 |

---

## 3. 项目结构

```
FatigueReconPrj-mumu/
├── PstechProject/                          # 主项目目录
│   ├── CMakeLists.txt                      # C++ 编译配置
│   ├── appsettings.json                    # 应用运行配置
│   ├── src/
│   │   ├── native/                         # C++ 原生代码 (~1209行)
│   │   │   ├── common/                     # 通用基础库 (ICamera, RingBuffer)
│   │   │   ├── plugins/                    # 相机驱动插件 (Hik/Dvt/Usb/Virtual)
│   │   │   ├── pstrack/                    # PsTrack 人脸识别包装
│   │   │   ├── export/                     # C 导出接口 (供 C# 调用)
│   │   │   └── tests/                      # 单元测试
│   │   └── managed/                        # C# .NET 8.0 代码 (~1039行)
│   │       ├── PstechManaged.slnx          # VS 解决方案
│   │       ├── Pstech.App/                 # 主应用入口
│   │       ├── Pstech.Core/                # P/Invoke 本地互操作
│   │       └── Pstech.Logic/               # 业务逻辑层
│   ├── sdk/                                # 第三方 SDK
│   │   ├── hikvision/                      # 海康威视 SDK
│   │   ├── dvt/                            # DVT 摄像机 SDK
│   │   └── visage/                         # Visage PsTrack 人脸库
│   ├── build_linux/                        # Linux 构建输出
│   └── output/                             # 发布产物
├── Build-Manager-V75.ps1                   # C++/C# 统一构建脚本
└── Deploy-Run-All-V78.ps1                  # WSL 部署和运行脚本
```

---

## 4. 分层架构

```
┌─────────────────────────────────────────┐
│         Pstech.App (应用层)              │  CLI 入口 + 配置加载 + 主循环
├─────────────────────────────────────────┤
│         Pstech.Logic (业务逻辑层)        │
│   ├─ PipelineAnalyzer   核心分析管道     │
│   ├─ YoloDetector       YOLO 目标检测    │
│   ├─ FatigueAnalyzer    疲劳状态分析     │
│   ├─ AlarmManager       多级警报上报     │
│   ├─ FrameDrawer        OSD 可视化绘制   │
│   └─ MjpegServer        Web GUI 服务     │
├─────────────────────────────────────────┤
│         Pstech.Core (互操作层)           │  P/Invoke 接口 + 数据结构映射
├─────────────────────────────────────────┤
│         PstechNative (C++ 原生库)        │
│   ├─ ICamera            抽象相机接口     │
│   ├─ CameraFactory      工厂模式创建     │
│   ├─ HikCamera          海康威视驱动     │
│   ├─ DvtCamera          DVT/大华驱动     │
│   ├─ UsbCamera          USB 摄像头       │
│   ├─ VirtualCamera      视频文件回放     │
│   ├─ PsTrackWrapper     人脸识别封装     │
│   └─ RingBuffer         无锁帧缓冲      │
└─────────────────────────────────────────┘
```

### 设计模式

| 模式 | 应用位置 | 说明 |
|------|---------|------|
| 工厂模式 | `CameraFactory` | 根据配置创建不同类型相机实例 |
| 策略模式 | `ICamera` | 不同相机驱动实现统一接口 |
| 生产者-消费者 | `RingBuffer` | 相机采集线程 → 分析线程 |
| 模板方法 | `PipelineAnalyzer` | 定义标准化分析流程 |

---

## 5. 核心功能模块

### 5.1 相机驱动系统

通过工厂模式支持四种视频源：

| 类型 | 类名 | 解码方式 | 典型用途 |
|------|------|---------|---------|
| `file` | VirtualCamera | OpenCV 读取 | 开发调试 |
| `usb` | UsbCamera | OpenCV VideoCapture | 本地摄像头 |
| `hik` | HikCamera | SDK 回调 + PlayCtrl | 海康 IP 摄像机 |
| `dvt` | DvtCamera | SDK 回调 + FFmpeg H.264 | DVT/大华 IP 摄像机 |

**RingBuffer**：150 帧循环缓冲（~135MB @640×480），原子操作无锁设计，保证实时性。

### 5.2 人脸跟踪 (PsTrack)

基于 Visage PsTrack 库，输出数据：

| 数据项 | 说明 |
|--------|------|
| `eyeClosure[2]` | 左右眼闭合度 [0.0-1.0]，< 0.5 判定为闭眼 |
| `headPose[3]` | Pitch/Yaw/Roll 头部姿态角（弧度） |
| `faceRect[4]` | 人脸检测框 [x, y, w, h] |
| `landmarks[468×2]` | 468 点人脸关键点坐标 |

### 5.3 YOLO 目标检测

- **模型**：YOLOv8n（nano 轻量版），ONNX 格式
- **检测类别**：人 (ClassId=0)、手机 (ClassId=67)
- **推理流程**：图像缩放 640×640 → RGB 归一化 → ONNX Runtime 推理 → NMS 去重（IoU > 0.45）
- **ROI 区域**：画面 10%-90% 宽度 × 20%-100% 高度

### 5.4 疲劳分析管道 (PipelineAnalyzer)

按优先级分析：

```
优先级 0 (最高)  ─  黑屏检测（帧亮度 < 阈值）
                     → 重置所有计时器，立即报警

优先级 1         ─  离岗检测（无人脸 + 无目标）
                     → 重置疲劳/分心计时，累积离岗时间

优先级 2         ─  疲劳检测（双眼闭合 > 阈值时间）
                 ─  分心检测（头部姿态角度 > 阈值时间）
                     → 两者并行检测
```

### 5.5 多级报警系统 (AlarmManager)

**报警类型**：

| AlarmType | 触发条件 | 默认阈值 |
|-----------|---------|---------|
| `Fatigue` | 双眼持续闭合 | 2000ms |
| `Distraction` | 头部姿态异常（Pitch>20° / Yaw>30° / Roll>20°） | 2000ms |
| `Absent` | 人脸与目标均消失 | 5000ms |
| `CameraError` | 画面黑屏 | 1000ms |
| `PhoneCall` | 手机检测（预留） | 2000ms |

**报警等级升级**：

```
Level1  ─  持续时间 > 阈值               (首次触发)
Level2  ─  持续时间 > 阈值 × 1.5         (中级)
Level3  ─  持续时间 > 阈值 × 2.0         (严重)
```

上报方式：HTTP POST → 配置的 API 地址，包含类型/等级/时长/截图(Base64)。

### 5.6 Web 可视化 (MjpegServer + FrameDrawer)

- **协议**：MJPEG（Motion JPEG），端口 8080
- **帧率**：20 FPS
- **OSD 信息**：实时状态、配置参数、报警提示框

---

## 6. 工作流程

### 6.1 启动流程

```
1. 加载配置 ─── appsettings.json
2. 命令行覆盖 ─ CLI 参数优先
3. 启动 Web GUI ─ MjpegServer 监听 :8080
4. 初始化相机 ── Native.Pstech_Camera_Init (C++ 层)
5. 启动采集 ──── Native.Pstech_Camera_Start
6. 初始化 AI ─── Native.Pstech_Alg_Init (PsTrack)
7. 进入主循环 ── 30ms 定时 (~33FPS)
8. Ctrl+C ────── 优雅关闭
```

### 6.2 主循环数据流

```
┌──────────┐     RingBuffer      ┌──────────────┐
│ 相机采集  │ ─── 无锁写入 ───→  │  帧读取       │
│ (C++ 线程)│                    │  (C# 主循环)  │
└──────────┘                    └──────┬───────┘
                                       │
                    ┌──────────────────┼───────────────────┐
                    │                  │                   │
             ┌──────▼──────┐   ┌──────▼──────┐   ┌──────▼──────┐
             │ YOLO 检测    │   │ PsTrack     │   │ 黑屏检测     │
             │ (每1帧)      │   │ (每3帧)      │   │ (每帧)       │
             │ 人/手机定位  │   │ 眼睛+头部    │   │ 亮度分析     │
             └──────┬──────┘   └──────┬──────┘   └──────┬──────┘
                    │                  │                   │
             ┌──────▼──────────────────▼───────────────────▼──────┐
             │              PipelineAnalyzer.Update()              │
             │   黑屏 → 离岗 → 疲劳/分心  (按优先级)               │
             └──────────────────────┬─────────────────────────────┘
                                    │
                     ┌──────────────┼──────────────┐
                     │                             │
              ┌──────▼──────┐              ┌──────▼──────┐
              │ AlarmManager │              │ FrameDrawer  │
              │ HTTP 上报     │              │ OSD 绘制     │
              │ 本地截图保存  │              │ MJPEG 推流   │
              └─────────────┘              └─────────────┘
```

### 6.3 命令行用法

```bash
# 视频文件回放（开发调试）
./Pstech.App file "../../test.mp4"

# USB 摄像头
./Pstech.App usb 0

# 海康威视 IP 摄像机
./Pstech.App hik "192.168.1.100" "admin" "123456" 1

# DVT/大华 IP 摄像机
./Pstech.App dvt "192.168.1.101" "admin" "123456" 1
```

---

## 7. 配置说明 (appsettings.json)

### 相机配置

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `Camera.Type` | `file` | 相机类型：file / usb / hik / dvt |
| `Camera.ConnectionString` | — | 文件路径或 IP 地址 |
| `Camera.User` / `Password` | — | IP 摄像机认证 |
| `Camera.Width` / `Height` | 640 / 480 | 分辨率 |
| `Camera.CheckBlack` | `true` | 启用黑屏检测 |
| `Camera.BrightnessThreshold` | `10.0` | 黑屏亮度阈值 |

### 算法配置

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `Algorithm.FatigueTimeMs` | `2000` | 疲劳触发时间（毫秒） |
| `Algorithm.DistractionTimeMs` | `2000` | 分心触发时间 |
| `Algorithm.AbsenceTimeMs` | `5000` | 离岗触发时间 |
| `Algorithm.BlackScreenTimeMs` | `1000` | 黑屏触发时间 |
| `Algorithm.MaxPitch` / `Roll` / `Yaw` | 20 / 20 / 30 | 头部角度阈值（度） |
| `Algorithm.OdModel` | `data/yolov8n.onnx` | YOLO 模型路径 |
| `Algorithm.OdInterval` | `1` | YOLO 检测间隔（帧数） |
| `Algorithm.PsTrackInterval` | `3` | 人脸跟踪间隔（帧数） |

### 上报配置

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `Report.Enable` | `true` | 启用报警上报 |
| `Report.ApiUrl` | `http://127.0.0.1:5000/api/alarm` | 上报接口 |
| `Report.Level2Ratio` | `1.5` | Level2 升级倍率 |
| `Report.Level3Ratio` | `2.0` | Level3 升级倍率 |

---

## 8. 构建与部署

### C++ 编译

通过 CMake 构建共享库 `PstechNative.so`，链接海康/DVT SDK、Visage（静态）、OpenCV、FFmpeg、OpenMP。

### C# 编译

.NET 8.0 项目，目标平台 `linux-x64`，启用 `AllowUnsafeBlocks`（指针操作）。

### 一键构建

```powershell
# 全量构建
.\Build-Manager-V75.ps1 -Target All

# 仅 C++
.\Build-Manager-V75.ps1 -Target Cpp

# 仅 C#
.\Build-Manager-V75.ps1 -Target CSharp
```

### 部署到 WSL

```powershell
.\Deploy-Run-All-V78.ps1
```

---

## 9. 性能指标

| 指标 | 规格 |
|------|------|
| 目标帧率 | 30 FPS |
| 分辨率 | 640×480（可配） |
| 帧缓冲 | 150 帧 / ~135 MB |
| YOLO 推理延迟 | < 100ms |
| 报警上报延迟 | < 3s |
| 核心代码量 | ~2,650 行 |

---

## 10. 核心源文件索引

| 文件 | 行数 | 职责 |
|------|------|------|
| `src/managed/Pstech.App/Program.cs` | 135 | 应用入口、配置加载、主循环 |
| `src/managed/Pstech.Logic/PipelineAnalyzer.cs` | 178 | 核心分析管道 |
| `src/managed/Pstech.Logic/YoloDetector.cs` | 138 | YOLO 推理与 NMS |
| `src/managed/Pstech.Logic/FrameDrawer.cs` | 138 | 可视化 OSD 绘制 |
| `src/managed/Pstech.Logic/AlarmManager.cs` | 74 | 多级警报上报 |
| `src/managed/Pstech.Core/NativeMethods.cs` | 59 | P/Invoke 接口定义 |
| `src/native/plugins/HikCamera.cpp` | 114 | 海康威视相机驱动 |
| `src/native/plugins/DvtCamera.cpp` | 103 | DVT 相机驱动 |
| `src/native/plugins/UsbCamera.cpp` | 92 | USB 摄像头驱动 |
| `src/native/plugins/VirtualCamera.cpp` | 42 | 视频文件回放 |
| `src/native/pstrack/PsTrackWrapper.cpp` | 95 | 人脸跟踪封装 |
| `src/native/common/ICamera.h` | 102 | 相机抽象接口 |
| `src/native/common/RingBuffer.h` | 83 | 无锁帧缓冲 |
| `src/native/export/PstechExport.cpp` | 45 | C++ 导出函数 |

## 实施计划：DVT 摄像头自动识别 H.264/H.265

### 任务类型
- [x] 后端（C++ 原生摄像头插件）
- [ ] 前端
- [ ] 全栈

### 增强后的需求

将 DVT SDK 回调的 Annex B 裸视频流从“固定按 H.264 解码”改为自动识别 H.264/AVC 与 H.265/HEVC。识别必须适用于带 28 字节 DVT 私有头和无私有头的现有数据路径，不改变 `DvtCamera` 对外接口及最终 `CV_8UC3/BGR24` 帧格式。遇到尚无足够特征的数据时不得误选 codec；SDK 重连后应允许重新识别，以支持设备码流配置发生变化。

### 现状与范围边界

- `DvtCamera::ProcessStreamData` 已剥离可选的 28 字节私有头，并只把以 Annex B 四字节起始码开头的数据交给 `FfmpegDecoder`。
- `FfmpegDecoder::init` 当前通过 `AV_CODEC_ID_H264` 写死 H.264，且 `reset` 只清理解码缓冲，不会重新选择 codec。
- `FfmpegDecoder` 仅被 DVT 插件调用，因此改造不会影响海康、USB 或虚拟摄像头。
- 本次不增加摄像头编码配置项，不修改 C ABI、C# 或前端，也不改变 BGR 输出契约。
- 本次不引入 `libavformat` 探测依赖，Windows 当前只链接 `avcodec/avutil/swscale`。

### 方案对比

| 方案 | 优点 | 缺点 | 结论 |
|------|------|------|------|
| 扫描 Annex B NAL 参数集 | SPS/PPS/VPS 类型可确定 codec；无额外依赖；每包开销极小 | 首次参数集到达前需要等待 | 推荐 |
| `av_probe_input_format*` 探测 | 复用 FFmpeg probe | 短回调包探测不稳定；Windows 需新增 `avformat` 链接 | 不采用 |
| 同时向 H.264/H.265 解码器试投 | 无需显式 NAL 规则 | 双倍状态/CPU；错误日志多；成功时机不确定 | 不采用 |

### 技术方案

在 `FfmpegDecoder` 中扫描 Annex B NAL 单元，同时兼容三字节和四字节起始码。只使用参数集进行强判定：

- H.264：以通过基础语法校验的 NAL type 7（SPS）为强特征；仅接受 `nal_ref_idc=3` 的典型 type 8（PPS）作为补充特征，避免与 H.265 type-20 IDR 混淆。
- H.265：NAL type 32（VPS）、33（SPS）或 34（PPS），并校验两字节 HEVC NAL header 的 `temporal_id_plus1 != 0`。
- 仅有普通 slice、数据不完整或格式非法时返回 unknown，等待后续参数集，避免启用错误解码器。

检测到 codec 后按需调用 `avcodec_find_decoder`、`avcodec_alloc_context3` 和 `avcodec_open2`。已选择 codec 时继续复用上下文；若后续参数集明确表明编码发生变化，则安全释放旧 codec 与 `SwsContext` 后切换。`reset` 用于 SDK 重连时清理解码状态并恢复 unknown，使新码流可重新识别。像素转换继续以实际解码帧格式为源，输出 `AV_PIX_FMT_BGR24/CV_8UC3`。

### 实施步骤

1. 在 `FfmpegDecoder.h` 增加轻量的 Annex B 起始码遍历和 codec 判定函数，覆盖三/四字节起始码、截断数据与非法 HEVC header。
2. 将固定 H.264 初始化重构为延迟初始化：`init()` 准备自动识别状态，`decode()` 在识别成功后打开对应 H.264/H.265 decoder。
3. 完善 codec 生命周期：同编码复用、明确编码切换时重建、SDK `reset()` 后重新识别，并清理与分辨率/像素格式相关的 `SwsContext`。
4. 保持 `DvtCamera` 的 28 字节头剥离与 `decode(...)` 调用契约不变；仅在需要时补充可诊断日志，输出识别到的 codec 或 decoder 缺失原因。
5. 增加原生小型测试，覆盖 H.264 SPS/PPS、H.265 VPS/SPS/PPS、三/四字节起始码、多 NAL、普通 slice 未知、截断/非法数据等判定场景。
6. 更新 Linux CMake 测试目标，构建受影响目标并运行识别测试；若本机缺少 Linux DVT SDK 运行条件，至少完成可用平台的编译或独立 detector 编译测试，并明确剩余设备联调项。
7. 审查线程安全、重连切换、资源释放、FFmpeg API 返回值和 BGR 输出不变性，修复发现的问题后重跑验证。

### 关键文件

| 文件 | 操作 | 说明 |
|------|------|------|
| `PstechProject/src/native/plugins/FfmpegDecoder.h` | 修改 | NAL 识别、H.264/H.265 延迟初始化、切换与资源管理 |
| `PstechProject/src/native/plugins/DvtCamera.cpp` | 核对/按需修改 | 保持 DVT 私有头剥离和解码入口兼容，必要时补充诊断 |
| `PstechProject/src/native/tests/test_ffmpeg_decoder.cpp` | 新增 | 自动识别边界测试 |
| `PstechProject/CMakeLists.txt` | 修改 | 注册 Linux 原生识别测试目标 |

### 风险与缓解

| 风险 | 缓解措施 |
|------|----------|
| 普通 slice 的 NAL type 在两种编码间存在位级歧义 | 仅凭 SPS/PPS/VPS 强特征选择 codec，不用 slice 猜测 |
| 回调首包不含参数集，短时间无画面 | 保持 unknown 并等待参数集；没有参数集时解码器本来也无法建立完整解码状态 |
| SDK 重连后编码从 H.264 改为 H.265 | `reset()` 恢复自动识别状态，收到新参数集后重新初始化 |
| 切换 codec 时残留 `SwsContext` 或尺寸状态 | codec 切换统一释放解码上下文、帧、包和缩放上下文，重置尺寸/像素格式缓存 |
| FFmpeg 构建缺少 HEVC decoder | 打开 codec 时明确报错并返回失败，不伪装解码成功 |
| Windows 链接依赖变化 | 不使用 `avformat`，沿用现有 `avcodec/avutil/swscale` 链接集合 |

### 验收标准

- [x] 输入含 H.264 SPS/PPS 时自动选择 `AV_CODEC_ID_H264`。
- [x] 输入含 H.265 VPS/SPS/PPS 时自动选择 `AV_CODEC_ID_HEVC`。
- [x] 同时支持 Annex B 三字节与四字节起始码的内部 NAL 扫描。
- [x] 不完整数据、普通 slice 或未知格式不会被误判为 H.264/H.265。
- [x] SDK 重连调用 `reset()` 后可重新识别，包括编码格式发生变化的情况。
- [x] 解码成功后的输出仍为 `CV_8UC3`、BGR 通道顺序。
- [x] 无 C ABI、C#、前端和其他摄像头插件行为变化。
- [x] 新增自动识别测试通过，受影响原生目标构建通过。
- [ ] 使用真实 DVT 设备分别用 H.264/H.265 码流完成预览联调。

### 验证方式

- 运行新增的 codec detector 原生测试，覆盖正常、复合与非法 NAL 样本。
- 构建 `PstechNative` 与新增测试目标，确认 FFmpeg H.264/HEVC API 在项目版本上兼容。
- 对变更执行 `git diff --check` 和人工自审计。
- 设备联调：将 DVT 对应码流分别配置为 H.264、H.265，确认日志识别正确、持续出帧、重连后恢复且图像颜色正常。

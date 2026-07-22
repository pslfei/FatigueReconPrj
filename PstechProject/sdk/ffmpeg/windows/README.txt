FFmpeg 4.4 (Windows x64, shared / dev build)
========================================================
来源:     https://github.com/GyanD/codexffmpeg/releases/tag/4.4
          (ffmpeg-4.4-full_build-shared.7z, gyan.dev 官方 GitHub 镜像)
下载日期: 2026-06-21
用途:     DVT 通路 FfmpegDecoder.h 依赖 (FFmpeg 4.x API + swscale)

目录结构:
  include/      头文件 (libavcodec/ libavutil/ libswscale/ libavformat/ ...)
  lib/          导入库 (.lib)
    avcodec.lib avutil.lib swscale.lib   <- FfmpegDecoder.h 实际只用这三个
    (avformat/avdevice/avfilter/postproc/swresample.lib 一并提供, 项目未用)
  bin/          运行时 DLL (部署需复制到 PstechNative.dll 同级)
    avcodec-58.dll  avutil-56.dll  swscale-5.dll  swresample-3.dll   <- 必需
    (avformat-58 / avdevice-58 / avfilter-7 / postproc-55.dll 可选)

CMake 接入 (Windows 分支, 待后续改 CMakeLists 时使用):
  include_directories(${CMAKE_SOURCE_DIR}/sdk/ffmpeg/windows/include)
  target_link_libraries(PstechNative
    ${CMAKE_SOURCE_DIR}/sdk/ffmpeg/windows/lib/avcodec.lib
    ${CMAKE_SOURCE_DIR}/sdk/ffmpeg/windows/lib/avutil.lib
    ${CMAKE_SOURCE_DIR}/sdk/ffmpeg/windows/lib/swscale.lib)

DLL 版本号对照 (FFmpeg 4.4):
  avcodec-58 / avutil-56 / swscale-5 / swresample-3

注意:
  - DVT SDK 自带 avcodec-56.dll (FFmpeg 2.6) 仅供 NsdNetSDK_x64.dll 内部使用,
    与此处 avcodec-58.dll (FFmpeg 4.4) dll 名不同, 运行时共存不冲突。
  - 运行时 avcodec-58 依赖 avutil-56 + swresample-3, 三者须一并部署。

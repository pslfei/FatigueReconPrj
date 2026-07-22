OpenCV 4.10.0 (Windows x64, VC16 / 适用 VS2019-2022)
========================================================
来源:     https://github.com/opencv/opencv/releases/tag/4.10.0
          (opencv-4.10.0-windows.exe 官方预编译自解压包)
下载日期: 2026-06-21
用途:     项目主体 OpenCV4 依赖 (Linux 端对应系统 opencv4)

目录结构:
  include/                            头文件 (find_package 用, 354 个)
  x64/vc16/lib/
    opencv_world4100.lib              Release 链接库
    opencv_world4100d.lib             Debug 链接库
    OpenCVConfig.cmake                find_package(OpenCV) 配置入口
    OpenCVModules*.cmake
  x64/vc16/bin/                       运行时 DLL (部署需复制到 PstechNative.dll 同级)
    opencv_world4100.dll              主库 (必需)
    opencv_videoio_ffmpeg4100_64.dll  视频文件读取后端 (VirtualCamera file 通路)
    opencv_videoio_msmf4100_64.dll    MSMF 后端 (USB 摄像头 CAP_MSMF)

CMake 接入 (Windows 分支, 待后续改 CMakeLists 时使用):
  set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/sdk/opencv/windows/x64/vc16")
  find_package(OpenCV REQUIRED)

备注:
  - 已精简: 删除 .pdb 调试符号、.exe 工具、debug 运行时 dll (部署用 Release)。
  - debug 链接库 opencv_world4100d.lib 保留, 供 find_package 的 Debug 配置使用。
  - 与各摄像头 SDK 自带的 opencv_*2411.dll (OpenCV 2.4.11) dll 名不同, 可共存。

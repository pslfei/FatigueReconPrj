cmake_minimum_required(VERSION 3.10)
project(PstechFatigueSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DNDEBUG)

# ========== 平台相关编译配置 ==========
if(WIN32)
    # Windows / MSVC：Visage 走动态(定制)库[路线B]，禁止定义 VISAGE_STATIC，
    # 否则头文件按无修饰展开，与导入库的 __declspec(dllimport) 不匹配导致链接失败。
    # 部分旧厂商 SDK 头文件不是 UTF-8；C4828 只反映其注释编码，避免产生海量无关警告。
    add_compile_options(/utf-8 /O2 /EHsc /wd4819 /wd4828)
else()
    # Linux / GCC：Visage 静态库
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
    add_definitions(-DVISAGE_STATIC -DLINUX)
    add_compile_options(-fpermissive -w -O2 -fopenmp)
endif()

# ========== 依赖查找 ==========
if(WIN32)
    # OpenCV4 与 FFmpeg4 由项目内置（sdk/），见各自 README
    set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/sdk/opencv/windows/x64/vc16/lib")
endif()

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
if(NOT WIN32)
    find_package(ZLIB REQUIRED)
endif()

# ========== FFmpeg ==========
if(WIN32)
    set(FFMPEG_INC "${CMAKE_SOURCE_DIR}/sdk/ffmpeg/windows/include")
    set(FFMPEG_LIBS
        "${CMAKE_SOURCE_DIR}/sdk/ffmpeg/windows/lib/avcodec.lib"
        "${CMAKE_SOURCE_DIR}/sdk/ffmpeg/windows/lib/avutil.lib"
        "${CMAKE_SOURCE_DIR}/sdk/ffmpeg/windows/lib/swscale.lib")
else()
    find_library(AVCODEC_LIB avcodec)
    find_library(AVFORMAT_LIB avformat)
    find_library(AVUTIL_LIB avutil)
    find_library(SWSCALE_LIB swscale)
    set(FFMPEG_LIBS ${AVCODEC_LIB} ${AVFORMAT_LIB} ${AVUTIL_LIB} ${SWSCALE_LIB})
    set(FFMPEG_INC "")
endif()

include_directories(
    src/native/common src/native/plugins src/native/pstrack
    ${OpenCV_INCLUDE_DIRS}
    ${FFMPEG_INC}
    sdk/hikvision/include sdk/dvt/include sdk/visage/include
)

# ========== 第三方 SDK 库（平台分支） ==========
if(WIN32)
    set(HIK_LIB_PATH    "${CMAKE_SOURCE_DIR}/sdk/hikvision/lib/windows")
    set(DVT_LIB_PATH    "${CMAKE_SOURCE_DIR}/sdk/dvt/lib/windows")
    set(VISAGE_LIB_PATH "${CMAKE_SOURCE_DIR}/sdk/visage/lib/windows")
    # 海康：导入库（运行时需配套 dll）
    set(HIK_LIBS
        "${HIK_LIB_PATH}/HCNetSDK.lib"
        "${HIK_LIB_PATH}/HCCore.lib"
        "${HIK_LIB_PATH}/PlayCtrl.lib"
        "${HIK_LIB_PATH}/GdiPlus.lib")
    # DVT：导入库
    set(DVT_LIBS "${DVT_LIB_PATH}/NsdNetSDK_x64.lib")
    # Visage：标准版导入库（运行时用 libVisageVision64.dll，由 Pstech 定制打包/授权）
    set(VISAGE_LIBS
        "${VISAGE_LIB_PATH}/libVisageVision64.lib"
        "${VISAGE_LIB_PATH}/libVisageAnalyser64.lib"
        "${VISAGE_LIB_PATH}/libVisageGaze64.lib")
    set(PLATFORM_LIBS "")
else()
    set(HIK_LIB_PATH    "${CMAKE_SOURCE_DIR}/sdk/hikvision/lib/linux")
    set(DVT_LIB_PATH    "${CMAKE_SOURCE_DIR}/sdk/dvt/lib/linux")
    set(VISAGE_LIB_PATH "${CMAKE_SOURCE_DIR}/sdk/visage/lib/linux")
    link_directories(/usr/lib/x86_64-linux-gnu /usr/local/lib)
    set(HIK_LIBS "${HIK_LIB_PATH}/libhcnetsdk.so" "${HIK_LIB_PATH}/libPlayCtrl.so")
    set(DVT_LIBS "${DVT_LIB_PATH}/libNsdNetSDK.so")
    set(VISAGE_LIBS "${VISAGE_LIB_PATH}/libVisageVision.a")
    set(PLATFORM_LIBS ${ZLIB_LIBRARIES} ${CMAKE_DL_LIBS})
endif()

# ========== 核心库 ==========
file(GLOB_RECURSE LIB_SOURCES "src/native/common/*.cpp" "src/native/plugins/*.cpp" "src/native/pstrack/*.cpp" "src/native/export/PstechExport.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*tests/.*")

add_library(PstechNative SHARED ${LIB_SOURCES})
target_link_libraries(PstechNative
    ${HIK_LIBS} ${DVT_LIBS} ${VISAGE_LIBS}
    OpenMP::OpenMP_CXX
    ${OpenCV_LIBS} ${FFMPEG_LIBS} Threads::Threads
    ${PLATFORM_LIBS}
)

enable_testing()
add_executable(Test_FfmpegDecoder src/native/tests/test_ffmpeg_decoder.cpp)
target_link_libraries(Test_FfmpegDecoder ${OpenCV_LIBS} ${FFMPEG_LIBS})
add_test(NAME FfmpegDecoderCodecDetection COMMAND Test_FfmpegDecoder)

# ========== 测试可执行（仅 Linux；链接 .so，Windows 暂不构建） ==========
if(NOT WIN32)
    set(COMMON_SRC src/native/common/ICamera.cpp src/native/plugins/CameraFactory.cpp)
    set(CV_LIBS opencv_core opencv_highgui opencv_videoio opencv_imgcodecs opencv_imgproc)

    add_executable(Test_Hik src/native/tests/test_hik.cpp ${COMMON_SRC} src/native/plugins/HikCamera.cpp)
    target_compile_definitions(Test_Hik PRIVATE TEST_ONLY_HIK TEST_MODE)
    target_link_libraries(Test_Hik ${CV_LIBS} Threads::Threads ${CMAKE_DL_LIBS} "${HIK_LIB_PATH}/libhcnetsdk.so" "${HIK_LIB_PATH}/libPlayCtrl.so")

    add_executable(Test_Dvt src/native/tests/test_dvt.cpp ${COMMON_SRC} src/native/plugins/DvtCamera.cpp)
    target_compile_definitions(Test_Dvt PRIVATE TEST_ONLY_DVT TEST_MODE)
    target_link_libraries(Test_Dvt ${CV_LIBS} ${FFMPEG_LIBS} Threads::Threads ${CMAKE_DL_LIBS} "${DVT_LIB_PATH}/libNsdNetSDK.so")

    add_executable(Test_Usb src/native/tests/test_usb.cpp ${COMMON_SRC} src/native/plugins/UsbCamera.cpp)
    target_compile_definitions(Test_Usb PRIVATE TEST_ONLY_USB TEST_MODE)
    target_link_libraries(Test_Usb ${CV_LIBS} Threads::Threads ${CMAKE_DL_LIBS})

    add_executable(Test_File src/native/tests/test_file.cpp ${COMMON_SRC} src/native/plugins/VirtualCamera.cpp)
    target_compile_definitions(Test_File PRIVATE TEST_ONLY_FILE TEST_MODE)
    target_link_libraries(Test_File ${CV_LIBS} Threads::Threads ${CMAKE_DL_LIBS})

    # 这里的 Test_Visage 已经改名为 Test_PsTrack
    add_executable(Test_PsTrack src/native/tests/test_pstrack.cpp src/native/pstrack/PsTrackWrapper.cpp)
    target_link_libraries(Test_PsTrack
        ${VISAGE_LIBS}
        OpenMP::OpenMP_CXX ${ZLIB_LIBRARIES} ${CMAKE_DL_LIBS}
        ${CV_LIBS} Threads::Threads
    )
endif()

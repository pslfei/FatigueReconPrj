# FatigueReconPrj

疲劳检测原生库项目，提供海康（Hikvision）、DVT、USB、视频文件及 Pstech/Visage 疲劳算法的统一 C++ 接口。仓库同时包含构建所需的第三方 SDK 头文件、链接库、运行库和算法模型，克隆后无需再单独寻找这些厂商依赖。

## 仓库内容

- `PstechProject/src/native/`：C++17 源码和测试程序源码。
- `PstechProject/sdk/`：OpenCV、FFmpeg、Hikvision、DVT、Visage/Pstech 的 Windows/Linux 依赖。
- `PstechProject/CMakeLists.txt`：跨平台 CMake 工程入口。
- `Build-Windows.ps1`：Windows x64 推荐的一键构建入口。
- `Build-Linux.sh`：Linux x64 构建入口。
- `PstechProject/test_run/*.py`：旧版 Windows 运行验证脚本；大体积 DLL 和模型副本不重复提交，它们的源文件位于 `sdk/`。

`build_win/`、`build_linux/`、`verify_build/`、`out/`、`output/` 和 `test_run/` 下的部署文件都是可再生成的产物，已从 Git 中排除。这样既保留全部源码和编译依赖，也避免将同一套 SDK 重复提交多次。

## 获取代码

```powershell
git clone https://github.com/pslfei/FatigueReconPrj.git
cd FatigueReconPrj
```

本仓库没有使用 Git LFS；普通 Git 克隆会直接取得完整 SDK，不需要额外执行 `git lfs pull`。

## Windows x64 编译（推荐）

### 只需安装一次的工具

1. Windows 10/11 x64。
2. Visual Studio 2022 Community、Professional 或 Enterprise。
3. 在 Visual Studio Installer 中勾选“使用 C++ 的桌面开发”，并安装 MSVC v143、Windows 10/11 SDK 和 CMake tools for Windows。
4. Git for Windows。

OpenCV 4.10、FFmpeg 4.4 以及三个厂商 SDK 已在仓库中，无需通过 vcpkg、NuGet 或其他网站另行下载。

### 一键编译

在仓库根目录打开 PowerShell：

```powershell
powershell -ExecutionPolicy Bypass -File .\Build-Windows.ps1
```

脚本会优先使用 `PATH` 中的 CMake；若未配置 `PATH`，会自动查找 Visual Studio 2022 自带的 CMake。默认生成 Release x64 版本：

```text
PstechProject/out/build/windows-x64/Release/PstechNative.dll
PstechProject/out/build/windows-x64/Release/PstechNative.lib
```

Debug 构建：

```powershell
powershell -ExecutionPolicy Bypass -File .\Build-Windows.ps1 -Configuration Debug
```

如需在 Visual Studio 中修改和调试，可先执行一次脚本，然后打开：

```text
PstechProject/out/build/windows-x64/PstechFatigueSystem.sln
```

源代码改动应保存在 `PstechProject/src/native/`，不要修改 `out/` 中由 CMake 生成的工程文件。

## Linux x64 编译

Linux 端的 Hikvision、DVT 和 Visage/Pstech 厂商库已包含在 `sdk/` 中；OpenCV、FFmpeg、zlib、编译器和 CMake 使用系统开发包。Debian/Ubuntu 可执行：

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libopencv-dev \
  libavcodec-dev libavformat-dev libavutil-dev libswscale-dev zlib1g-dev
./Build-Linux.sh
```

产物位于：

```text
PstechProject/out/build/linux-x64/libPstechNative.so
```

Linux 厂商二进制库对系统 ABI 有要求；项目历史验证环境为 Debian 11（bullseye）/x86_64。若使用更新发行版，建议优先在 Debian 11 容器或 WSL 环境中构建和运行。

## 依赖说明

| 依赖 | Windows | Linux | 仓库位置 |
| --- | --- | --- | --- |
| OpenCV 4 | 已内置 | 系统 `libopencv-dev` | `PstechProject/sdk/opencv/` |
| FFmpeg 4 | 已内置 | 系统开发包 | `PstechProject/sdk/ffmpeg/` |
| Hikvision SDK | 已内置 | 已内置 | `PstechProject/sdk/hikvision/` |
| DVT SDK | 已内置 | 已内置 | `PstechProject/sdk/dvt/` |
| Visage/Pstech SDK 与模型 | 已内置 | 已内置 | `PstechProject/sdk/visage/` |

编译器、Windows SDK、CMake 和 Linux 基础系统库属于操作系统工具链，不适合直接放进源码仓库；上面的安装步骤固定了所需组件。所有项目专用、难以从公共包管理器恢复的依赖均已随代码提交。

## 运行时注意事项

编译成功只生成项目自身的动态库。部署应用时，还需把对应平台的厂商运行库、OpenCV/FFmpeg DLL 或 SO、以及 `sdk/visage/data/` 模型复制到应用目录，并配置相机驱动、网络地址和厂商授权。已有的 `Deploy-Run-All-V78.ps1` 与 `PstechProject/test_run/*.py` 可作为旧部署流程参考；它们不是完成编译所必需的步骤。

仓库中的厂商 SDK 和模型可能受各自商业授权条款约束。将仓库设为公开仓库或向第三方分发前，请确认账号和组织拥有相应再分发权限。

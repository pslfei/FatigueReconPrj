# =============================================================================
# Pstech 综合构建管理脚本 V75
# 功能：统一管理 C++ (Docker) 和 C# (.NET) 的编译流程
# 用法：
#   .\Build-Manager-V75.ps1           -> 进入交互菜单
#   .\Build-Manager-V75.ps1 -Target All    -> 编译所有
#   .\Build-Manager-V75.ps1 -Target Cpp    -> 只编译 C++
#   .\Build-Manager-V75.ps1 -Target CSharp -> 只编译 C#
#   .\Build-Manager-V75.ps1 -Target Clean  -> 清理所有缓存
# =============================================================================

param (
    [string]$Target = "Menu" # 默认显示菜单
)

$ErrorActionPreference = "Stop"
$ProjectName = "PstechProject"
$BaseDir = Join-Path (Get-Location) $ProjectName

if (-not (Test-Path $BaseDir)) {
    Write-Error "❌ 错误：找不到项目目录 '$BaseDir'。请先运行生成脚本。"
    exit 1
}

# =============================================================================
# 核心构建函数
# =============================================================================

function Clean-Artifacts {
    Write-Host "`n[Action] 正在清理构建缓存..." -ForegroundColor Yellow
    
    $PathsToRemove = @(
        "$BaseDir\build_linux",
        "$BaseDir\src\managed\Pstech.App\bin",
        "$BaseDir\src\managed\Pstech.App\obj",
        "$BaseDir\output"
    )

    foreach ($p in $PathsToRemove) {
        if (Test-Path $p) {
            Remove-Item $p -Recurse -Force
            Write-Host "  Deleted: $p" -ForegroundColor Gray
        }
    }
    Write-Host "✅ 清理完成。" -ForegroundColor Green
}

function Build-Cpp {
    Write-Host "`n[Action] 开始编译 C++ (Docker: dvt-builder:bullseye)..." -ForegroundColor Cyan
    
    # 确保输出目录存在
    $OutDir = Join-Path $BaseDir "build_linux"
    if (-not (Test-Path $OutDir)) { New-Item -Path $OutDir -ItemType Directory | Out-Null }

    # 调用 Docker 编译
    # 逻辑：挂载当前目录 -> 进入 build_linux -> cmake -> make
    $DockerCmd = "mkdir -p build_linux && cd build_linux && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j4"
    
    try {
        docker run --rm -v "${BaseDir}:/app" -w /app dvt-builder:bullseye /bin/bash -c $DockerCmd
        
        # 验证产物
        $SoPath = Join-Path $BaseDir "build_linux\libPstechNative.so"
        if (Test-Path $SoPath) {
            Write-Host "✅ C++ 编译成功: libPstechNative.so" -ForegroundColor Green
        } else {
            throw "C++ 编译似乎完成了，但没找到 .so 文件。"
        }
    } catch {
        Write-Error "❌ C++ 编译失败: $_"
        exit 1
    }
}

function Build-CSharp {
    Write-Host "`n[Action] 开始编译 C# (.NET 8.0)..." -ForegroundColor Cyan
    
    $CsProj = Join-Path $BaseDir "src\managed\Pstech.App\Pstech.App.csproj"
    $PublishDir = Join-Path $BaseDir "output\bin\linux\managed"
    
    # 1. 编译
    dotnet publish $CsProj -c Release -r linux-x64 --self-contained false -o $PublishDir
    
    if ($LASTEXITCODE -ne 0) { throw "dotnet publish 失败" }

    # 2. 部署依赖 (C++ 库)
    Write-Host "  -> 同步依赖文件..." -ForegroundColor Gray
    $SoSrc = Join-Path $BaseDir "build_linux\libPstechNative.so"
    if (Test-Path $SoSrc) {
        Copy-Item $SoSrc $PublishDir -Force
        Write-Host "     [Lib] libPstechNative.so (OK)" -ForegroundColor Gray
    } else {
        Write-Warning "⚠️ 警告: 找不到 C++ 库 ($SoSrc)。程序运行时会报错！建议先编译 C++。"
    }

    # 3. 部署数据 (Data)
    $DataDst = Join-Path $PublishDir "data"
    if (-not (Test-Path $DataDst)) { New-Item -Path $DataDst -ItemType Directory | Out-Null }
    
    # 3.1 拷贝 SDK 数据 (模型/配置)
    $SdkData = Join-Path $BaseDir "sdk\visage\data"
    if (Test-Path $SdkData) {
        Copy-Item "$SdkData\*" $DataDst -Recurse -Force
    }

    # 3.2 拷贝字体 (从 build_linux 或 sdk)
    $FontSrc1 = Join-Path $BaseDir "build_linux\data\OpenSans.ttf"
    $FontSrc2 = Join-Path $BaseDir "sdk\visage\data\OpenSans.ttf"
    if (Test-Path $FontSrc1) { Copy-Item $FontSrc1 $DataDst -Force }
    elseif (Test-Path $FontSrc2) { Copy-Item $FontSrc2 $DataDst -Force }

    # 3.3 拷贝 appsettings.json
    $ConfigSrc = Join-Path $BaseDir "src\managed\Pstech.App\appsettings.json"
    if (Test-Path $ConfigSrc) { Copy-Item $ConfigSrc $PublishDir -Force }

    Write-Host "✅ C# 编译与部署成功: $PublishDir" -ForegroundColor Green
}

# =============================================================================
# 主逻辑
# =============================================================================

# 如果没有参数，显示交互菜单
if ($Target -eq "Menu") {
    Clear-Host
    Write-Host "==========================================" -ForegroundColor Cyan
    Write-Host "   Pstech 构建管理器 (V75)" -ForegroundColor Cyan
    Write-Host "=========================================="
    Write-Host " 1. [All]    编译全部 (C++ + C#)"
    Write-Host " 2. [Cpp]    只编译 C++ Native"
    Write-Host " 3. [CSharp] 只编译 C# Managed"
    Write-Host " 4. [Clean]  清理所有缓存"
    Write-Host " Q. 退出"
    Write-Host "=========================================="
    
    $Selection = Read-Host "请选择 (1-4)"
    switch ($Selection) {
        "1" { $Target = "All" }
        "2" { $Target = "Cpp" }
        "3" { $Target = "CSharp" }
        "4" { $Target = "Clean" }
        "Q" { exit 0 }
        "q" { exit 0 }
        Default { Write-Error "无效选择"; exit 1 }
    }
}

# 执行选定的目标
switch ($Target) {
    "Clean" {
        Clean-Artifacts
    }
    "Cpp" {
        Build-Cpp
    }
    "CSharp" {
        Build-CSharp
    }
    "All" {
        Write-Host ">>> 启动全量构建..." -ForegroundColor Magenta
        Build-Cpp
        Build-CSharp
        Write-Host "`n🎉 全量构建完成！" -ForegroundColor Magenta
    }
    Default {
        Write-Error "未知目标: $Target. 可选: All, Cpp, CSharp, Clean"
    }
}
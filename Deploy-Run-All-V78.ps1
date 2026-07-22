# =============================================================================
# Pstech 一键同步与运行脚本 V78
# 功能：
#   1. 同步 C++ 和 C# 的所有编译产物到 WSL
#   2. 生成 universal_run.sh，包含硬编码的相机参数
#   3. 配置完整的运行时环境
# =============================================================================

$ProjectName = "PstechProject"
$BaseDir = Join-Path (Get-Location) $ProjectName
if (-not (Test-Path $BaseDir)) { Write-Error "找不到项目目录 $BaseDir"; exit }

$WslSourcePath = (wsl -e wslpath -a "$BaseDir").Trim()
Write-Host "[1] 源路径: $WslSourcePath" -ForegroundColor Gray

# 使用单引号模板，避免 PowerShell 变量解析冲突
$BashDeployTemplate = @'
#!/bin/bash
SOURCE_DIR="{{SOURCE_DIR}}"
DEST_DIR=~/pstech_test
MANAGED_DEST="$DEST_DIR/csharp_app"

echo "=========================================="
echo "   Pstech 全量同步 V78"
echo "=========================================="

# 1. 准备目录
mkdir -p "$DEST_DIR/build_linux/data"
mkdir -p "$DEST_DIR/sdk"
mkdir -p "$MANAGED_DEST/data"

# 2. 同步 C++ 产物
echo "[Sync] C++ Binaries..."
if [ -d "$SOURCE_DIR/build_linux" ]; then
    # 拷贝可执行文件和库
    find "$SOURCE_DIR/build_linux" -maxdepth 1 -name "Test_*" -exec cp -f {} "$DEST_DIR/build_linux/" \;
    find "$SOURCE_DIR/build_linux" -maxdepth 1 -name "*.so" -exec cp -f {} "$DEST_DIR/build_linux/" \;
    
    # 拷贝 C++ 库到 C# 目录 (C# 运行依赖)
    find "$SOURCE_DIR/build_linux" -maxdepth 1 -name "*.so" -exec cp -f {} "$MANAGED_DEST/" \;
else
    echo "⚠️ Warning: C++ build_linux not found."
fi

# 3. 同步 C# 产物
echo "[Sync] C# Managed App..."
CSHARP_SRC="$SOURCE_DIR/output/bin/linux/managed"
if [ -d "$CSHARP_SRC" ]; then
    cp -rf "$CSHARP_SRC/"* "$MANAGED_DEST/"
else
    echo "⚠️ Warning: C# output directory not found. Run Build-Manager first."
fi

# 4. 同步 SDK 库
echo "[Sync] SDK Libraries..."
if [ -d "$SOURCE_DIR/sdk" ]; then
    cp -rn "$SOURCE_DIR/sdk/"* "$DEST_DIR/sdk/" 2>/dev/null || true
fi

# 5. 资源文件处理 (Font & Config)
echo "[Sync] Resources..."
# 字体
if [ ! -f "$MANAGED_DEST/data/OpenSans.ttf" ]; then
    find "$SOURCE_DIR" -name "OpenSans.ttf" -exec cp {} "$MANAGED_DEST/data/" \; -quit
fi
# 配置
if [ ! -f "$MANAGED_DEST/data/pstrack.cfg" ]; then
    # 优先找改名后的，再找原始名
    if [ -f "$SOURCE_DIR/build_linux/data/pstrack.cfg" ]; then
        cp "$SOURCE_DIR/build_linux/data/pstrack.cfg" "$MANAGED_DEST/data/"
    elif [ -f "$SOURCE_DIR/sdk/visage/data/Facial Features Tracker - With Ears.cfg" ]; then
        cp "$SOURCE_DIR/sdk/visage/data/Facial Features Tracker - With Ears.cfg" "$MANAGED_DEST/data/pstrack.cfg"
    fi
fi
# 确保 C++ 测试用的数据也存在
cp -r "$MANAGED_DEST/data/"* "$DEST_DIR/build_linux/data/" 2>/dev/null || true

# 6. 生成通用启动脚本


echo "✅ 部署完成: ~/pstech_test/universal_run.sh"
'@

# 注入变量 & 处理换行
$BashScript = $BashDeployTemplate.Replace("{{SOURCE_DIR}}", $WslSourcePath)
$BashScript = $BashScript -replace "`r", ""

$TempWinPath = [System.IO.Path]::GetTempFileName()
[System.IO.File]::WriteAllText($TempWinPath, $BashScript, (New-Object System.Text.UTF8Encoding $false))
$TempWslPath = (wsl -e wslpath -a "$TempWinPath").Trim()

Write-Host "[2] 执行同步..." -ForegroundColor Cyan
wsl -e bash "$TempWslPath"
Remove-Item $TempWinPath

Write-Host "`n[+] 同步完成！请在 WSL 中使用以下命令：" -ForegroundColor Green
Write-Host "  cd ~/pstech_test"
Write-Host "  ./universal_run.sh dvt      (测 DVT)"
Write-Host "  ./universal_run.sh hik      (测 海康)"
Write-Host "  ./universal_run.sh pstrack  (测 算法)"
Write-Host "  ./universal_run.sh cs_file  (测 C#程序)"
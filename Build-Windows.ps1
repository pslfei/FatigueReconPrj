[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

$RepositoryRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$SourceDirectory = Join-Path $RepositoryRoot "PstechProject"
$BuildDirectory = Join-Path $SourceDirectory "out\build\windows-x64"

function Find-CMake {
    $Command = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($Command) {
        return $Command.Source
    }

    $VsWhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $VsWhere) {
        $VisualStudioPath = & $VsWhere -latest -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath

        if ($VisualStudioPath) {
            $BundledCMake = Join-Path $VisualStudioPath.Trim() `
                "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
            if (Test-Path -LiteralPath $BundledCMake) {
                return $BundledCMake
            }
        }
    }

    throw "CMake and the Visual Studio C++ toolchain were not found. Install 'Desktop development with C++' and 'CMake tools for Windows' with Visual Studio Installer."
}

$CMake = Find-CMake
Write-Host "CMake: $CMake"
Write-Host "Source: $SourceDirectory"
Write-Host "Build:  $BuildDirectory"

& $CMake -S $SourceDirectory -B $BuildDirectory `
    -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed with exit code $LASTEXITCODE."
}

& $CMake --build $BuildDirectory --config $Configuration --parallel
if ($LASTEXITCODE -ne 0) {
    throw "CMake build failed with exit code $LASTEXITCODE."
}

$Library = Join-Path $BuildDirectory "$Configuration\PstechNative.dll"
if (-not (Test-Path -LiteralPath $Library)) {
    throw "The build completed but the expected output was not found: $Library"
}

Write-Host ""
Write-Host "Build succeeded: $Library" -ForegroundColor Green

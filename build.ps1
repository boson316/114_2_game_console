# 建置 game_console_hub（避免 cmake --build 卡在 Re-running CMake）
$ErrorActionPreference = "Stop"
$Root = $PSScriptRoot
Set-Location $Root

function Find-CmakeExe {
    if (Get-Command cmake -ErrorAction SilentlyContinue) {
        return (Get-Command cmake).Source
    }
    $vs = "${env:ProgramFiles}\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if (Test-Path $vs) { return $vs }
    throw "找不到 cmake"
}

$cmake = Find-CmakeExe
$buildDir = Join-Path $Root "build"
$ninja = Get-Command ninja -ErrorAction SilentlyContinue

if (-not (Test-Path (Join-Path $buildDir "build.ninja"))) {
    Write-Host "== 首次設定 build（請耐心等 raylib 下載） =="
    & $cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release `
        -DCMAKE_CXX_COMPILER="C:/msys64/ucrt64/bin/g++.exe"
}

Write-Host "== 編譯（直接用 ninja，不觸發 Re-running CMake） =="
if ($ninja) {
    Push-Location $buildDir
    & ninja game_console_hub
    Pop-Location
} else {
    & $cmake --build build --target game_console_hub
}

$exe = Join-Path $buildDir "hub\game_console_hub.exe"
if (Test-Path $exe) {
    Write-Host "OK: $exe"
} else {
    Write-Host "找不到執行檔，請檢查上方錯誤"
}

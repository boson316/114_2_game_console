# 建置 bomberman（自動找 cmake；缺字型時下載台北黑體）
param(
    [string]$Target = "bomberman_gui"
)

$ErrorActionPreference = "Stop"
$Root = $PSScriptRoot

function Find-CmakeExe {
    if (Get-Command cmake -ErrorAction SilentlyContinue) {
        return (Get-Command cmake).Source
    }
    $roots = @(
        "${env:ProgramFiles}\Microsoft Visual Studio",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio"
    )
    foreach ($base in $roots) {
        if (-not (Test-Path $base)) { continue }
        $found = Get-ChildItem -Path $base -Filter cmake.exe -Recurse -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake\.exe$" } |
            Select-Object -First 1
        if ($found) { return $found.FullName }
    }
    throw @"
找不到 cmake。請擇一：
  1) Visual Studio Installer → 修改 → 勾選「C++ CMake tools for Windows」
  2) 將 cmake 加入 PATH 後重開 PowerShell
  3) 手動指定：
     & 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build --target bomberman_gui
"@
}

$font = Join-Path $Root "assets\fonts\TaipeiSansTCBeta-Regular.ttf"
if (-not (Test-Path $font)) {
    Write-Host "UI font missing; running scripts/fetch_ui_font.py ..."
    python (Join-Path $Root "scripts\fetch_ui_font.py")
}

$cmake = Find-CmakeExe
$buildDir = Join-Path $Root "build"
if (-not (Test-Path $buildDir)) {
    Write-Host "configure: $cmake -B build ..."
    & $cmake -B $buildDir -G Ninja -DCMAKE_BUILD_TYPE=Release
}
Write-Host "build: $cmake --build build --target $Target"
& $cmake --build $buildDir --target $Target

# 一鍵準備字型 + 美術 + 建置 GUI
$ErrorActionPreference = "Stop"
$Root = $PSScriptRoot | Split-Path -Parent
Set-Location $Root

Write-Host "== Taipei Sans TC =="
python "$Root\scripts\fetch_ui_font.py"

Write-Host "== Sprites =="
python "$Root\scripts\generate_assets.py"

if (Test-Path "$Root\build.ps1") {
    & "$Root\build.ps1"
} else {
    Write-Host "Run build.ps1 manually after installing CMake."
}

# UI 字型：台北黑體

遊戲 UI 預設使用 **台北黑體 Taipei Sans TC**（SIL OFL 1.1，[翰字鑄造 JT Foundry](https://sites.google.com/view/jtfoundry/zh-tw/downloads)）。

## 第一次建置前

```powershell
python scripts/fetch_ui_font.py
```

會下載 `TaipeiSansTCBeta-Regular.ttf`（約 20MB，建議勿 commit 進 Git）。

## 載入順序（程式內）

1. `assets/fonts/TaipeiSansTCBeta-Regular.ttf`
2. 系統已安裝的台北黑體（`C:\Windows\Fonts\`）
3. Noto Sans TC（若存在）
4. 標楷體 `kaiu.ttf`（僅備援）

## 授權

字型依 **SIL Open Font License 1.1** 散佈；改作自思源黑體。遊戲內嵌僅供本專題展示，公開 repo 請保留本說明並勿聲稱字型著作權為己有。

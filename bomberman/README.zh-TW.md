# 💣 Bomberman（爆爆王風格）

**Languages:** [English](README.md) · [中文](README.zh-TW.md)

Bomberman 是一款 **網格式炸彈人**（爆爆王玩法風格，**非官方美術**）的 C++ 專案：支援 Raylib GUI 與終端機 ASCII。含 **BFS 路徑 AI**、**Q-learning 敵人策略**、連鎖爆炸、道具成長與本機／實驗性 LAN 連線。

本目錄為 **114-2 遊戲主控台** 整合版之一環；可經根目錄 `hub/game_console_hub` 啟動，亦可單獨建置 `bomberman_gui`。

**授權：** 僅課程／學術用途 — 見 [`LICENSE`](LICENSE)。

---

## ✨ 遊戲願景與特色

- **經典網格戰鬥**：15×13 地圖、十字爆炸、可破壞軟牆、固定硬牆與出生安全區。
- **連鎖與道具**：炸彈連鎖引爆；軟牆毀滅有機率掉落 **炸彈數／火力／移速** 三種強化（各有上限）。
- **智慧敵人 AI**：以 **BFS** 追擊玩家；危險區 **優先躲彈**；積極模式可攻擊放彈；**每 5 秒至多破一塊軟牆**（需安全逃離路徑）。
- **強化學習（可選）**：預設載入 `assets/ai/enemy_qtable.json`；可用 `scripts/train_enemy_rl.py` 重新訓練 Q-table。
- **多模式**：單人、本機雙人（P1 WASD + 空白／P2 方向鍵 + Enter）、實驗性 **LAN 開房／加入**（TCP **7755**）。
- **Hub 整合**：從主選單進入時為 **1344×756** 視窗；結算可 **Play Again**／**Main Menu** 返回遊戲主控台（避免缺字，Hub 模式按鈕為英文標籤）。
- **美術合規**：素材由 `scripts/generate_assets.py` 腳本生成，**禁止**使用遊戲天堂／爆爆王官圖。

---

## 🛠️ 開發環境要求

| 項目 | 說明 |
|------|------|
| 語言標準 | **C++20** |
| 建置系統 | **CMake 3.16+** |
| 編譯器 | MSVC（Visual Studio 2022）／ **GCC**（建議 MSYS2 UCRT64）／ Clang |
| 圖形庫 | **Raylib 5.5**（整合 repo 根目錄由 CMake `FetchContent` 下載 tarball，無須手動安裝） |
| UI 字型 | 台北黑體等（首次請執行 `scripts/fetch_ui_font.py`） |
| 測試 | `bomberman_tests`（Catch2 風格單元／屬性測試） |

---

## 🚀 建置與執行說明

### A. 整合主控台（推薦，含西洋棋 + 爆爆王）

在 **repo 根目錄** `114_2_game_console-main/`：

```powershell
# 1. 首次：下載 UI 字型（爆爆王／Hub 共用）
cd bomberman
python scripts/fetch_ui_font.py
cd ..

# 2. 建置（建議用專案腳本或 ninja，路徑含中文時避免 cmake --build 重跑 CMake）
.\build.ps1
# 或：cd build && ninja game_console_hub

# 3. 執行主選單
.\build\hub\game_console_hub.exe
```

**主選單操作：** `1` 西洋棋戰爭 · `2` Bomberman · **`Esc` 離開程式**（子遊戲結束後會回到主選單）。

> **注意：** 第一次 `cmake -B build` 可能需 5–15 分鐘下載 Raylib。已 configure 過請優先 `cd build && ninja game_console_hub`，勿頻繁 `cmake --build` 以免卡在 *Re-running CMake*。

---

### B. 僅建置爆爆王（本目錄）

```powershell
# 1. 進入 bomberman 目錄
cd bomberman

# 2. 下載字型（首次必做）
python scripts/fetch_ui_font.py

# 3a. 一鍵（字型 + 美術 + 建置）
.\scripts\setup_game.ps1

# 3b. 或手動（整合 repo 請在根目錄 build/ 指定 target）
cd ..\build
ninja bomberman_gui
```

**執行：**

```powershell
.\build\bomberman\bomberman_gui.exe   # GUI（推薦）
.\build\bomberman\bomberman.exe         # 終端機 ASCII
.\build\bomberman\bomberman_tests.exe # 測試
```

**重新訓練敵人 RL（可選）：**

```powershell
cd bomberman
python scripts/train_enemy_rl.py
```

---

## 🎮 操作（GUI）

| 模式 | 說明 |
|------|------|
| 單人 | 1P vs RL／啟發式 AI |
| 雙人 | 同機：P1 WASD+空白；P2 方向鍵+Enter |
| 開房／加入 | LAN TCP **7755**（實驗性） |

| 按鍵 | 功能 |
|------|------|
| 選單 | 難度、模式、字體 ±（90–150%）、開始遊戲 |
| P1 | WASD + 空白（放彈） |
| P2 | 方向鍵 + Enter |
| 重開本局 | R |
| 回選單 | Esc（Hub 模式下 Esc 可返回主控台） |

設定與最高分：`save/profile.save`（已 gitignore）。

---

## 📋 規則摘要

| 類別 | 重點 |
|------|------|
| 勝利 | 消滅所有敵人 |
| 失敗 | 被爆炸或敵人碰到 |
| 得分 | 每殺一敵 +100 |
| 難度 | 簡單 1 敵／普通 2 敵／困難 3 敵（移動、放彈、掉寶率不同） |
| 完整參數 | `include/GameConfig.hpp`、`PRD.md` |

---

## 📁 目錄結構

| 路徑 | 說明 |
|------|------|
| `src/Game.cpp` | 核心邏輯、回合狀態 |
| `src/GfxApp.cpp` | Raylib 選單／對戰／結算 |
| `src/GfxAssets.cpp` | 字型、貼圖、音效載入 |
| `src/AI_Enemy.cpp` | BFS + RL 敵人行為 |
| `assets/sprites/` | 腳本生成精靈圖（見 `assets/CREDITS.md`） |
| `assets/ai/enemy_qtable.json` | 預訓練 Q-table |
| `tests/` | `bomberman_tests` |
| `scripts/` | 字型、美術、RL 訓練腳本 |

上架前檢查：`PUBLISH_CHECKLIST.md`。**勿提交** `build/`、`save/`、`assets/fonts/*.ttf`（clone 後執行 `fetch_ui_font.py`）。

---

## 🔗 相關文件

- 整合 repo 說明：[`../README.zh-TW.md`](../README.zh-TW.md)
- 西洋棋模組：[`../chess_war/`](../chess_war/)（同學協作）
- 獨立公開副本（非主線）：`bomberman-cpp-final/`

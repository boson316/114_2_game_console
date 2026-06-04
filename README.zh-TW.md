# 🎮 114-2 Game Console (Chess War + Bomberman)

**Languages:** [English](README.md) · [中文](README.zh-TW.md)

114-2 Game Console 是一款 **C++ 期末整合專案**：以單一 **Raylib 主選單**（`game_console_hub`）串接兩款子遊戲——**Chess War**（西洋棋 × 卡牌 Roguelike）與 **Bomberman**（網格式炸彈人 + BFS／Q-learning AI）。根目錄 `CMakeLists.txt` 統一 FetchContent 下載 **Raylib 5.5**，Hub 與子遊戲共用同一套建置目錄。

---

## ✨ 遊戲願景與特色

### 主控台 Hub（`hub/`）

- **一鍵切換**：主選單按 **`1`** 進入 Chess War、**`2`** 進入 Bomberman、**`Esc`** 離開；子遊戲結束後回到主選單，無須重開程式。
- **共用 UI 字型**：Hub 與 Chess War 使用 `bomberman/assets/fonts/TaipeiSansTCBeta-Regular.ttf`（clone 後需執行字型腳本，`.ttf` 不進 Git）。

### ♟️ Chess War（`chess_war/`，同學協作）

Chess War 結合 **西洋棋移動規則**、**卡牌構築 (Deckbuilding)** 與 **Roguelike 成長** 的回合制戰術遊戲。

- **雙重行動點 (2 AP)**：每回合 2 點 AP，可分配給兩位英雄。
- **西洋棋子卡牌**：各英雄 11 張初始牌組（3×士兵、2×騎士、2×主教、2×城堡、1×國王、1×皇后）；手牌補滿 4 張；打出消耗 1 AP，可當 **移動** 或 **攻擊**。
- **動態西洋棋身份**：英雄初始無狀態，以棋子卡移動時切換對應身份至回合結束。
- **吃子與連技**：九宮格內擊殺位移；支援 **王車易位** 等西洋棋連技。
- **Roguelike 成長**：擊殺獲 XP，滿 5 XP 升級，可強化屬性或解鎖「士兵後退」、「騎士加長跳」、「主教穿透」等規則技能。

### 💣 Bomberman（`bomberman/`，本組）

網格式炸彈人：15×13 地圖、十字爆炸、連鎖、道具成長、**BFS 敵人 AI** 與可選 **Q-learning** 策略。詳見 [`bomberman/README.zh-TW.md`](bomberman/README.zh-TW.md)。

---

## 📁 專案結構

| 目錄 | 說明 |
|------|------|
| `hub/` | 主選單啟動器（`game_console_hub`） |
| `chess_war/` | Chess War 核心程式 |
| `bomberman/` | Bomberman GUI／AI／字型腳本 |

---

## 🛠️ 開發環境要求

| 項目 | 說明 |
|------|------|
| 語言標準 | **C++20** |
| 建置系統 | **CMake 3.16+** |
| 編譯器 | MSVC（Visual Studio 2022）／ GCC／ Clang |
| 圖形庫 | **Raylib 5.5**（根目錄 CMake `FetchContent` 自動下載 tarball，無須預裝） |
| UI 字型 | clone 後執行 `bomberman/scripts/fetch_ui_font.py`（約 20MB，不進 Git） |

---

## 🚀 建置與執行說明

### Windows（命令列 CMake，repo 根目錄）

```powershell
# 1. 切換至專案根目錄
cd 114_2_game_console-main

# 2. 首次：下載 UI 字型（Hub／Chess War 共用；未下載時中文可能顯示 ?）
python bomberman/scripts/fetch_ui_font.py

# 3. 設定建置目錄並編譯主選單
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target game_console_hub

# 4. 執行主控台
.\build\hub\game_console_hub.exe
```

**主選單：** **`1`** = Chess War · **`2`** = Bomberman · **`Esc`** = 離開。

**選用 target：** `chess_war`、`bomberman_gui`（同一 `build/` 目錄）。

---

## 📝 備註

- 子遊戲結束後需 **重新啟動 `game_console_hub`** 才能再選另一款（或從 Bomberman 結算畫面返回主選單，依子專案整合行為而定）。
- Bomberman 授權：僅課程／學術用途 — 見 [`bomberman/LICENSE`](bomberman/LICENSE)。

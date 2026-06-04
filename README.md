# 🎮 114-2 Game Console (Chess War + Bomberman)

**Languages:** [English](README.md) · [中文](README.zh-TW.md)

114-2 Game Console is a **C++ final integration project**: one **Raylib launcher** (`game_console_hub`) hosts **Chess War** (Western chess × deckbuilding roguelike) and **Bomberman** (grid bomber + BFS / Q-learning AI). The root `CMakeLists.txt` fetches **Raylib 5.5** via `FetchContent`; Hub and subgames share one `build/` tree.

---

## ✨ Vision & Features

### Hub launcher (`hub/`)

- **One menu, two games:** press **`1`** for Chess War, **`2`** for Bomberman, **`Esc`** to quit; return to the menu after a subgame exits.
- **Shared UI font:** Hub and Chess War use `bomberman/assets/fonts/TaipeiSansTCBeta-Regular.ttf` (run the fetch script after clone; `.ttf` files are not in Git).

### ♟️ Chess War (`chess_war/`, teammate)

Chess War blends **Western chess movement**, **deckbuilding**, and **roguelike progression** in a turn-based tactics game.

- **Dual action points (2 AP):** spend 2 AP per turn across two heroes.
- **Chess-piece cards:** each hero starts with 11 cards (3× pawn, 2× knight, 2× bishop, 2× rook, 1× king, 1× queen); hand refills to 4; playing costs 1 AP as **move** or **attack**.
- **Dynamic chess identity:** heroes start neutral; moving with a piece card switches identity until end of turn.
- **Captures & combos:** kill-and-step within range; supports **castling** and other chess-style chains.
- **Roguelike growth:** earn XP from kills; at 5 XP choose stat boosts or rule-breaking skills (pawn retreat, extended knight jump, bishop pierce, etc.).

### 💣 Bomberman (`bomberman/`, our team)

Grid bomber: 15×13 map, cross explosions, chains, power-ups, **BFS enemy AI**, optional **Q-learning**. See [`bomberman/README.md`](bomberman/README.md).

---

## 📁 Layout

| Folder | Role |
|--------|------|
| `hub/` | Launcher (`game_console_hub`) |
| `chess_war/` | Chess War core |
| `bomberman/` | Bomberman GUI / AI / font scripts |

---

## 🛠️ Requirements

| Item | Notes |
|------|-------|
| Language | **C++20** |
| Build | **CMake 3.16+** |
| Compiler | MSVC (VS 2022) / GCC / Clang |
| Graphics | **Raylib 5.5** (root CMake `FetchContent` tarball; no manual install) |
| UI font | run `bomberman/scripts/fetch_ui_font.py` after clone (~20MB, not in Git) |

---

## 🚀 Build & Run

### Windows (CLI CMake, repo root)

```powershell
# 1. Repo root
cd 114_2_game_console-main

# 2. First time: fetch UI font (shared by Hub / Chess War; missing font may show ?)
python bomberman/scripts/fetch_ui_font.py

# 3. Configure and build launcher
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target game_console_hub

# 4. Run
.\build\hub\game_console_hub.exe
```

**Menu:** **`1`** = Chess War · **`2`** = Bomberman · **`Esc`** = quit.

**Optional targets:** `chess_war`, `bomberman_gui` (same `build/`).

---

## 📝 Notes

- After a subgame exits you may need to **restart `game_console_hub`** to pick the other title (or use Bomberman’s return-to-menu flow when integrated).
- Bomberman license: educational use only — see [`bomberman/LICENSE`](bomberman/LICENSE).

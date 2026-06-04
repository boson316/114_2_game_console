# 114-2 Game Console (Chess War + Bomberman)

**Languages:** [English](README.md) · [中文](README.zh-TW.md)

Combined C++ final project: one **Raylib launcher** lets you pick **Chess War** (Western chess + cards) or **Bomberman** (grid bomber, BFS AI).

## Layout

| Folder | Role |
|--------|------|
| `hub/` | Launcher UI (`game_console_hub`) |
| `chess_war/` | Classmate — chess/card roguelike |
| `bomberman/` | Our team — bomber + RL enemies |

## Build

**Clone 後先下載 UI 字型**（`.ttf` 不進 Git，約 20MB）：

```powershell
cd <repo-root>
python bomberman/scripts/fetch_ui_font.py
```

Hub 與 Chess War 會共用 `bomberman/assets/fonts/TaipeiSansTCBeta-Regular.ttf`；若未下載，中文可能變 `?` 或退回系統字型。

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target game_console_hub
.\build\hub\game_console_hub.exe
```

Keys: **1** = Chess War, **2** = Bomberman, **Esc** = quit launcher.

Optional targets: `chess_war`, `bomberman_gui`.

## Notes

- Single Raylib 5.5 via root `CMakeLists.txt`.
- Restart `game_console_hub` to pick another game after one exits.
- Bomberman license: `bomberman/LICENSE` (educational use).

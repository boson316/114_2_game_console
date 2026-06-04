# Enemy RL policy

- **Training:** `python scripts/train_enemy_rl.py` → writes `enemy_qtable.json`
- **Runtime:** `Game` loads Q-table; `AI_Enemy::decide()` picks move + bomb (6 actions)
- **Fallback:** heuristic chase / dodge / bomb if JSON missing

**Languages:** [English](README.md) · [中文](../README.zh-TW.md)

#!/usr/bin/env python3
"""Tabular Q-learning for enemy: chase player spawn + live player, dodge, bomb."""
from __future__ import annotations

import argparse
import json
import random
from pathlib import Path

DX_BINS = 7
DY_BINS = 7
WALL_MASK_BINS = 16
STATE_COUNT = DX_BINS * DY_BINS * 2 * 2 * WALL_MASK_BINS
ACTION_COUNT = 6
ACTIONS = [(0, -1), (0, 1), (-1, 0), (1, 0), (0, 0), None]

PLAYER_SPAWN = (1, 1)
GRID_W, GRID_H = 15, 13
ENEMY_STARTS = [(13, 11), (11, 11), (13, 9), (9, 11)]


def encode(
    ex: tuple[int, int],
    pl: tuple[int, int],
    danger: bool,
    bomb_feet: bool,
    walls: int,
) -> int:
    dx = max(-3, min(3, pl[0] - ex[0])) + 3
    dy = max(-3, min(3, pl[1] - ex[1])) + 3
    return (
        dx
        + dy * DX_BINS
        + int(danger) * DX_BINS * DY_BINS
        + int(bomb_feet) * DX_BINS * DY_BINS * 2
        + walls * DX_BINS * DY_BINS * 4
    )


def wall_mask(ex: tuple[int, int], grid: list[list[int]]) -> int:
    h, w = len(grid), len(grid[0])
    mask = 0
    bit = 1
    for dx, dy in [(0, -1), (0, 1), (-1, 0), (1, 0)]:
        nx, ny = ex[0] + dx, ex[1] + dy
        blocked = nx < 0 or ny < 0 or nx >= w or ny >= h or grid[ny][nx] != 0
        if blocked:
            mask |= bit
        bit <<= 1
    return mask


def manhattan(a: tuple[int, int], b: tuple[int, int]) -> int:
    return abs(a[0] - b[0]) + abs(a[1] - b[1])


def chase_target(ex: tuple[int, int], pl: tuple[int, int]) -> tuple[int, int]:
    to_spawn = manhattan(ex, PLAYER_SPAWN)
    to_player = manhattan(ex, pl)
    return PLAYER_SPAWN if to_spawn < to_player else pl


def make_grid() -> list[list[int]]:
    g = [[0] * GRID_W for _ in range(GRID_H)]
    for y in range(GRID_H):
        for x in range(GRID_W):
            if x % 2 == 0 and y % 2 == 0:
                g[y][x] = 2
    safe = {PLAYER_SPAWN, (PLAYER_SPAWN[0] + 1, PLAYER_SPAWN[1]), (PLAYER_SPAWN[0], PLAYER_SPAWN[1] + 1)}
    for _ in range(22):
        x, y = random.randrange(GRID_W), random.randrange(GRID_H)
        if g[y][x] == 0 and (x, y) not in safe:
            g[y][x] = 1
    return g


def danger_cells(
    grid: list[list[int]], bombs: list[tuple[tuple[int, int], int]]
) -> set[tuple[int, int]]:
    cells: set[tuple[int, int]] = set()
    h, w = len(grid), len(grid[0])
    for (bx, by), radius in bombs:
        cells.add((bx, by))
        for dx, dy in [(0, -1), (0, 1), (-1, 0), (1, 0)]:
            for step in range(1, radius + 1):
                x, y = bx + dx * step, by + dy * step
                if x < 0 or y < 0 or x >= w or y >= h:
                    break
                cells.add((x, y))
                if grid[y][x] == 2:
                    break
                if grid[y][x] == 1:
                    cells.add((x, y))
                    break
    return cells


def step_env(
    grid: list[list[int]],
    ex: tuple[int, int],
    pl: tuple[int, int],
    bombs: list[tuple[tuple[int, int], int]],
    action: int,
) -> tuple[tuple[int, int], tuple[int, int], list[tuple[tuple[int, int], int]], float, bool]:
    reward = -0.04
    done = False
    goal = chase_target(ex, pl)

    if action == 5:
        if ex not in danger_cells(grid, bombs) and not any(b[0] == ex for b in bombs):
            bombs = bombs + [(ex, 2)]
            reward += 0.35
            if 1 <= manhattan(ex, pl) <= 3:
                reward += 0.55
        else:
            reward -= 0.35
    else:
        dx, dy = ACTIONS[action]
        old_d = manhattan(ex, goal)
        nx, ny = ex[0] + dx, ex[1] + dy
        if 0 <= nx < GRID_W and 0 <= ny < GRID_H and grid[ny][nx] == 0:
            ex = (nx, ny)
        new_d = manhattan(ex, goal)
        if new_d < old_d:
            reward += 0.55
        elif new_d > old_d:
            reward -= 0.22
        if action == 4:
            reward -= 0.18

    if random.random() < 0.08:
        dirs = [(0, -1), (0, 1), (-1, 0), (1, 0), (0, 0)]
        dx, dy = random.choice(dirs)
        nx, ny = pl[0] + dx, pl[1] + dy
        if 0 <= nx < GRID_W and 0 <= ny < GRID_H and grid[ny][nx] == 0:
            pl = (nx, ny)

    danger = danger_cells(grid, bombs)
    if ex in danger:
        reward -= 1.0
    if ex == pl:
        reward += 14.0
        done = True
    if ex in danger and random.random() < 0.12:
        reward -= 7.0
        done = True

    return ex, pl, bombs, reward, done


def greedy_policy(q: list[float], ex: tuple[int, int], pl: tuple[int, int], grid: list[list[int]], bombs: list) -> int:
    danger = ex in danger_cells(grid, bombs)
    bomb_feet = any(b[0] == ex for b in bombs)
    goal = chase_target(ex, pl)
    s = encode(ex, goal, danger, bomb_feet, wall_mask(ex, grid))
    base = s * ACTION_COUNT
    return max(range(ACTION_COUNT), key=lambda i: q[base + i])


def evaluate(q: list[float], episodes: int = 120) -> tuple[float, float]:
    successes = 0
    total_close = 0.0
    for _ in range(episodes):
        grid = make_grid()
        ex = random.choice(ENEMY_STARTS)
        pl = PLAYER_SPAWN
        bombs: list[tuple[tuple[int, int], int]] = []
        min_dist = manhattan(ex, pl)
        for _step in range(100):
            a = greedy_policy(q, ex, pl, grid, bombs)
            ex, pl, bombs, _r, done = step_env(grid, ex, pl, bombs, a)
            min_dist = min(min_dist, manhattan(ex, pl))
            if done:
                successes += 1
                break
        total_close += min_dist
    return successes / episodes, total_close / episodes


def train(episodes: int) -> list[float]:
    q = [0.0] * (STATE_COUNT * ACTION_COUNT)
    alpha, gamma = 0.2, 0.93
    eps = 0.35
    eps_end = 0.04

    for ep in range(episodes):
        grid = make_grid()
        ex = random.choice(ENEMY_STARTS)
        pl = PLAYER_SPAWN
        bombs: list[tuple[tuple[int, int], int]] = []
        eps = max(eps_end, eps * 0.9997)

        for _ in range(90):
            goal = chase_target(ex, pl)
            danger = ex in danger_cells(grid, bombs)
            bomb_feet = any(b[0] == ex for b in bombs)
            s = encode(ex, goal, danger, bomb_feet, wall_mask(ex, grid))
            if random.random() < eps:
                a = random.randrange(ACTION_COUNT)
            else:
                base = s * ACTION_COUNT
                a = max(range(ACTION_COUNT), key=lambda i: q[base + i])

            ex, pl, bombs, r, done = step_env(grid, ex, pl, bombs, a)
            goal2 = chase_target(ex, pl)
            danger2 = ex in danger_cells(grid, bombs)
            bomb_feet2 = any(b[0] == ex for b in bombs)
            s2 = encode(ex, goal2, danger2, bomb_feet2, wall_mask(ex, grid))
            base, base2 = s * ACTION_COUNT, s2 * ACTION_COUNT
            best_next = max(q[base2 + i] for i in range(ACTION_COUNT))
            q[base + a] += alpha * (r + gamma * best_next - q[base + a])
            if done:
                break
    return q


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--episodes", type=int, default=25000)
    parser.add_argument("--until-success", action="store_true")
    parser.add_argument("--target-rate", type=float, default=0.68)
    parser.add_argument("--max-rounds", type=int, default=8)
    args = parser.parse_args()

    random.seed(42)
    q: list[float] = []
    success = 0.0
    avg_close = 99.0
    rounds = 1 if not args.until_success else args.max_rounds

    for rnd in range(rounds):
        batch = args.episodes if not args.until_success else max(args.episodes, 10000)
        q = train(batch)
        success, avg_close = evaluate(q)
        print(f"round {rnd + 1}: kill_rate={success:.3f} avg_min_dist={avg_close:.2f}")
        if args.until_success and success >= args.target_rate:
            break

    out_dir = Path(__file__).resolve().parents[1] / "assets" / "ai"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "enemy_qtable.json"
    payload = {
        "version": 1,
        "state_count": STATE_COUNT,
        "action_count": ACTION_COUNT,
        "actions": ["up", "down", "left", "right", "wait", "bomb"],
        "q": [round(v, 5) for v in q],
    }
    out_path.write_text(json.dumps(payload, separators=(",", ":")), encoding="utf-8")
    print("wrote", out_path, "kill_rate", f"{success:.3f}", "avg_min_dist", f"{avg_close:.2f}")


if __name__ == "__main__":
    main()

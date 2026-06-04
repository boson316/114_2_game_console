#pragma once

enum class GameState { INIT, PLAYING, GAME_OVER, VICTORY, QUIT };

struct GameConfig {
    int gridWidth = 15;
    int gridHeight = 13;
    float destructibleDensity = 0.5f;
    int maxBombs = 1;
    int blastRadius = 2;
    float bombFuseTime = 3.0f;
    float explosionDuration = 0.5f;
    int enemyCount = 2;
    float enemyMoveInterval = 0.5f;
    float enemyBombFuseTime = 2.8f;
    int enemyBlastRadius = 2;
    float enemyBombCooldown = 2.2f;
    /** 怪物以炸彈破壞可破壞方塊的最小間隔（秒） */
    float enemyWallBreakInterval = 5.0f;
    float enemyBombPlaceChance = 0.42f;
    bool enemyUseRL = true;
    /** 困難：主動追擊、炸開阻擋的軟牆、較積極放彈 */
    bool enemyAggressive = false;
    /** 以玩家出生點為中心的交戰半徑（曼哈頓）；預設覆蓋全圖 */
    int enemyEngageRadius = 99;
    /** 僅腳下／爆炸格緊急躲彈；不對「鄰格有火」過度反應 */
    bool enemyDodgeAdjacentCells = false;
    /** 兩次緊急躲彈最小間隔（秒），接近一般人反應 */
    float enemyDodgeInterval = 0.42f;
    float powerupDropChance = 0.2f;
    int targetFPS = 60;

    static GameConfig clamped(GameConfig cfg);
};

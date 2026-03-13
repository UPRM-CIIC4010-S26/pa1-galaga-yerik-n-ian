#pragma once

#include <iostream>
#include <string>
#include "raylib-cpp/raylib-cpp.hpp"
#include "Background.hpp"
#include "Player.hpp"
#include "StdEnemy.hpp"
#include "SpEnemy.hpp"
#include "StEnemy.hpp"
#include "DyEnemy.hpp"
class Program {
    private:
        Background background = Background();
        Player* player = new Player((GetScreenWidth() / 2) - 15, GetScreenHeight() * 0.75f);
        Player* player2 = nullptr;
        int respawnCooldown = 620;
        int respawns = 0;
        int count = 0;
        int delay = 0;
        int livesP1 = 3;
        int livesP2 = 0;
        int pauseFrames = 0;
        int score = 0;
        int nextLifeScore = 1000;
        static constexpr int maxLives = 5;

        bool startup = true;
        bool paused = false;
        bool gameOver = false;
        bool coop_selected = false;
        bool coop_enabled = false;
        int selectedDifficulty = 1;
        int activeDifficulty = 1;
        bool clearProjectilesNextFrame = false;

    public:
        Program();
        
        void Update();
        void Draw();
        void ManageEnemyRespawns();
        void DrawStartup();
        void DrawPauseScreen();
        void DrawGameOver();
        void KeyInputs();
        void PlayerReset(Player* target);
        bool IsPlayerAlive(Player* target);
        void Reset();
        void AddScore(int points);

        ~Program() {}
};
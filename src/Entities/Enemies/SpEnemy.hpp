#pragma once
#include "Enemy.hpp"

class SpEnemy : public Enemy {
    private: 
        float angle = 90;
        int delay = 0;
        int specialCooldown = Enemy::DifficultyRoll(1800, 3000, 900, 1800, 420, 960);
        bool stop = false;

    public:
        SpEnemy(float x, float y) : Enemy(x, y) { 
            this->cooldown = Enemy::DifficultyRoll(540, 1200, 300, 720, 180, 420);
            this->health = 4;
            this->scoreValue = 300;
        }

        SpEnemy(float x, float y, bool newSpawn) : Enemy(x, y){
            this->cooldown = Enemy::DifficultyRoll(600, 1260, 360, 780, 210, 480);
            this->health = 4;
            this->scoreValue = 300;
            this->spawning = newSpawn;
        }

        void draw() override;
        void update(std::pair<float, float> pos, HitBox target) override;
        void attack(HitBox target) override;
};
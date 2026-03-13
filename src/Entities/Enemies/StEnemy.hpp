#pragma once
#include "Enemy.hpp"

class StEnemy : public Enemy {
    private: 
        float angle = 90;

    public:
        StEnemy(float x, float y) : Enemy(x, y) {  
            this->cooldown = Enemy::DifficultyRoll(180, 720, 120, 480, 45, 150);
            this->scoreValue = 150;
        }
        StEnemy(float x, float y, bool newSpawn) : Enemy(x, y) {     
            this->cooldown = Enemy::DifficultyRoll(210, 780, 150, 540, 60, 180);
            this->scoreValue = 150;
            this->spawning = newSpawn;
        }

        void draw() override;
        void update(std::pair<float, float> pos, HitBox target) override;
        void attack(HitBox target) override;
};
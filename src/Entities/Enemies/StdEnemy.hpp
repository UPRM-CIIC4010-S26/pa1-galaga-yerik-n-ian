#pragma once
#include "Enemy.hpp"

class StdEnemy : public Enemy {
    private:
        float angle = 90;
        int specialCooldown = Enemy::DifficultyRoll(1200, 2400, 600, 1500, 300, 900);
        int type = GetRandomValue(1, 2);
        inline static int attackCooldown = 300;

    public:
        inline static bool attackInProgress = false;

        StdEnemy(float x, float y) : Enemy(x, y) {
            this->cooldown = Enemy::DifficultyRoll(360, 960, 240, 720, 120, 360);
            this->health = 2;
            this->scoreValue = 100;
        }

        StdEnemy(float x, float y, bool newSpawn) : Enemy(x, y) {
            this->cooldown = Enemy::DifficultyRoll(420, 1020, 300, 780, 150, 420);
            this->health = 2;
            this->scoreValue = 100;
            this->spawning = newSpawn;
        }

        void draw() override;
        void update(std::pair<float, float> pos, HitBox target) override;
        void attack(HitBox target) override;

        static void attackReset() {
            const int resetValue = Enemy::DifficultyValue(420, 300, 180);

            if (attackInProgress) {
                attackCooldown--;
            } else {
                attackCooldown = resetValue;
            }

            if (attackCooldown <= 0) {
                attackInProgress = false;
                attackCooldown = resetValue;
            }
        }
};
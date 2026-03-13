#pragma once
#include "Projectile.hpp"
#include "Math.hpp"
#include "ImageManager.hpp"
#include "Animation.hpp"
#include <iostream>

class Enemy {
    protected:
        float angle = 90;
        int cooldown = 60;
        int speed = 3;
        bool spawning = false;
        bool frame = false;
        int frameCooldown = 30;
        
    public:
        int health = 1;
        int scoreValue = 0;
        std::pair<float, float> position;
        HitBox hitBox;

        inline static float direction = 0.5;
        inline static int directionChange = 100;
        inline static int difficultyLevel = 1; // 0 easy, 1 medium, 2 hard
        inline static std::vector<std::pair<std::pair<float, float>, Enemy*>> enemies;

        Enemy() {}

        Enemy(float x, float y) {
            position.first = x;
            position.second = y;
            hitBox = HitBox(x, y, 30, 30);
        }

        static int DifficultyValue(int easy, int medium, int hard) {
            if (difficultyLevel == 0) return easy;
            if (difficultyLevel == 2) return hard;
            return medium;
        }

        static int DifficultyRoll(int easyMin, int easyMax, int mediumMin, int mediumMax, int hardMin, int hardMax) {
            return GetRandomValue(
                DifficultyValue(easyMin, mediumMin, hardMin),
                DifficultyValue(easyMax, mediumMax, hardMax)
            );
        }

        virtual void draw() = 0;
        virtual void update(std::pair<float, float> pos, HitBox target) = 0;
        virtual void attack(HitBox target) = 0;

        void frameChange() {
            frameCooldown--;

            if (frameCooldown <= 0) {
                frame = !frame;
                frameCooldown = 30;
            }
        }

        static int ManageEnemies(HitBox target) {
            int pointsEarned = 0;
            for (std::pair<std::pair<float, float>, Enemy*>& p : Enemy::enemies) {
                p.first.first += (p.first.first == 0) ? 0 : direction;
                if (p.second) {
                    p.second->update(p.first, target);

                    for (Projectile& p2 : Projectile::projectiles) {
                        if (p2.ID != 1 && HitBox::Collision(p.second->hitBox, p2.getHitBox())) {
                            p.second->health--;
                            p2.del = true;
                            PlaySound(SoundManager::hit);
                        }
                    }

                    if (p.second->health <= 0) {
                        Animation::animations.push_back(
                            Animation(p.second->position.first, p.second->position.second, 155, 0, 33, 33, 30, 30, 4, ImageManager::SpriteSheet)
                        );
                        PlaySound(SoundManager::dead);
                        pointsEarned += p.second->scoreValue;
                        delete p.second;
                        p.second = nullptr;
                    }
                }
            }
            
            for (int i = 0; i < Enemy::enemies.size(); i++) {
                if ((Enemy::enemies[i].second && Enemy::enemies[i].second->position.first <= -30) || 
                    (!Enemy::enemies[i].second && Enemy::enemies[i].first.first == 0 && Enemy::enemies[i].first.second == 0)) {
                    Enemy::enemies.erase(Enemy::enemies.begin() + i);
                }
            }

            directionChange++;

            if (directionChange >= 200) {
                directionChange = 0;
                direction *= -1;
            }

            return pointsEarned;
        }
};
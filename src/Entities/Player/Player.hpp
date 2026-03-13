#pragma once
#include "Hitbox.hpp"
#include "Projectile.hpp"
#include "Background.hpp"
#include "ImageManager.hpp"

class Player {
    private:
        int cooldown = 30;
        int speed = 3;

    
    public:
        std::pair<float, float> position;
        HitBox hitBox;

        Player(float x, float y) {
            this->position.first = x;
            this->position.second = y;
            this->hitBox = HitBox(x, y, 30, 30);
        }

        void draw();
        void update();
        void keyInputs(int leftKey = 'A', int rightKey = 'D', int shootKey = KEY_SPACE);
        void attack();
};
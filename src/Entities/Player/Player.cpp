#include "Player.hpp"
#include "Enemy.hpp"

void Player::draw() {
    if (HitBox::drawHitbox) this->hitBox.draw();
    DrawTexturePro(ImageManager::SpriteSheet, Rectangle{0, 0, 17, 18}, 
                   Rectangle{this->position.first, this->position.second, 30, 30}, 
                   Vector2{0, 0}, 0, WHITE);
}

void Player::update() {
    this->hitBox.box.x = this->position.first;
    this->hitBox.box.y = this->position.second;
    this->cooldown--;

    if (HitBox::Collision(Background::sideWalls.first, this->hitBox)) {
        std::pair<double, double> collision = HitBox::CollisionMargins(this->hitBox, Background::sideWalls.first);
        if (abs(collision.second) < abs(collision.first)) {
            this->position.second += collision.second;
        } else {
            this->position.first += collision.first + 1;
        }
    }
    if (HitBox::Collision(Background::sideWalls.second, this->hitBox)) {
        std::pair<double, double> collision = HitBox::CollisionMargins(this->hitBox, Background::sideWalls.second);
        if (abs(collision.second) < abs(collision.first)) {
            this->position.second += collision.second;
        } else {
            this->position.first += collision.first + 1;
        }
    }
}

void Player::keyInputs(int leftKey, int rightKey, int shootKey) {
    if (IsKeyDown(leftKey)) this->position.first -= this->speed;
    if (IsKeyDown(rightKey)) this->position.first += this->speed;
    if (IsKeyDown(shootKey)) this->attack();
}

void Player::attack() {
    if (cooldown <= 0) {
        if(Enemy::difficultyLevel == 2){
        Projectile::projectiles.push_back(Projectile((position.first + this->hitBox.box.width / 2) - 10, this->position.second, 95, 0));
        Projectile::projectiles.push_back(Projectile((position.first + + this->hitBox.box.width / 2), position.second, 0));
        Projectile::projectiles.push_back(Projectile((position.first + this->hitBox.box.width / 2) + 10, this->position.second, 85, 0));
        PlaySound(SoundManager::shoot);
        cooldown = 20;

        }
        else{
        Projectile::projectiles.push_back(Projectile(position.first + + this->hitBox.box.width / 2, position.second, 0));
        PlaySound(SoundManager::shoot);
        cooldown = 30;
    }
}
}
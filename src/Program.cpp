#include "Program.hpp"
#include "Hitbox.hpp"

Program::Program()
{
    Background::sideWalls = std::pair<HitBox, HitBox>{
        HitBox(0, 0, 10, GetScreenHeight()),
        HitBox(GetScreenWidth() - 10, 0, 10, GetScreenHeight())};

    Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy *>{
        std::pair<float, float>{350, 150},
        new SpEnemy(350, 150)});

    Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy *>{
        std::pair<float, float>{600, 150},
        new SpEnemy(600, 150)});

    for (int i = 0; i < 30; i++)
    {
        float x = 250 + 50 * (i % 10);
        float y = 200 + 50 * (i / 10);

        Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy *>{
            std::pair<float, float>{x, y},
            new StdEnemy(x, y)});
    }
}

void Program::Update()
{
    for (Animation &a : Animation::animations)
        a.update();
    for (int i = 0; i < Animation::animations.size(); i++)
    {
        if (Animation::animations[i].done)
            Animation::animations.erase(Animation::animations.begin() + i);
    }
    pauseFrames = std::max(pauseFrames - 1, 0);

    if (!startup && !paused && !gameOver && pauseFrames <= 0)
    {
        AddScore(Enemy::ManageEnemies(player->hitBox));
        StdEnemy::attackReset();
        ManageEnemyRespawns();
        player->update();
        if (coop_enabled && player2 != nullptr)
            player2->update();

        for (std::pair<std::pair<float, float>, Enemy *> p : Enemy::enemies)
        {
            if (!p.second)
                continue;
            if (HitBox::Collision(player->hitBox, p.second->hitBox))
            {
                p.second->health = 0;
                PlayerReset(player);
            }
            if (coop_enabled && player2 != nullptr && HitBox::Collision(player2->hitBox, p.second->hitBox))
            {
                p.second->health = 0;
                PlayerReset(player2);
            }
        }

        for (Projectile &p : Projectile::projectiles)
        {
            p.update();
            if (p.ID != 0)
            {
                if (HitBox::Collision(player->hitBox, p.getHitBox()))
                {
                    p.del = true;
                    PlayerReset(player);
                }
                if (coop_enabled && player2 != nullptr && HitBox::Collision(player2->hitBox, p.getHitBox()))
                {
                    p.del = true;
                    PlayerReset(player2);
                }
            }
        }

        if (lives <= 0 && pauseFrames <= 0)
            gameOver = true;
        Projectile::CleanProjectiles();
        Projectile::ProjectileCollision();
    }
}

void Program::Draw()
{
    background.Draw();
    DrawText(TextFormat("Score: %i", score), 10, 10, 32, WHITE);
    if (pauseFrames <= 0 && !gameOver)
    {
        player->draw();

        if (coop_enabled && player2 != nullptr)
            player2->draw();
    }
    for (Animation &a : Animation::animations)
        a.draw();

    for (int i = 0; i < lives; i++)
    {
        DrawTexturePro(ImageManager::SpriteSheet, Rectangle{0, 0, 17, 18},
                       Rectangle{10.0f + i * 30, GetScreenHeight() - 30.0f, 20, 20},
                       Vector2{0, 0}, 0, WHITE);
    }

    for (Projectile p : Projectile::projectiles)
        p.draw();
    for (std::pair<std::pair<float, float>, Enemy *> &p : Enemy::enemies)
        if (p.second)
            p.second->draw();

    if (startup)
        DrawStartup();
    if (paused)
        DrawPauseScreen();
    if (gameOver)
        DrawGameOver();
}

void Program::ManageEnemyRespawns()
{
    delay = std::max(delay - 1, 0);

    int decay = 1 + (score / 2000);
    if (decay > 4)
        decay = 4;

    respawnCooldown = std::max(respawnCooldown - decay, 0);
    if (respawnCooldown <= 0)
    {
        respawnCooldown = 1080;
        for (std::pair<std::pair<float, float>, Enemy *> &p : Enemy::enemies)
        {
            if (!p.second && p.first.second != 150)
            {
                int eType = GetRandomValue(1, 3);

                if (eType == 1)
                {
                    p.second = new StEnemy(GetScreenWidth() / 2 - 15, 0, true);
                    respawnCooldown /= 2;
                }
                else
                {
                    p.second = new StdEnemy(GetScreenWidth() / 2 - 15, 0, true);
                }

                respawns++;
                break;
            }
            else if (!p.second && p.first.second == 150)
            {
                p.second = new SpEnemy(GetScreenWidth() / 2 - 15, 0, true);
                respawns++;
                break;
            }
        }
    }

    if (respawns >= 4)
    {
        count = 4;
        respawns = 0;
    }

    if (count > 0 && delay <= 0)
    {
        Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy *>{
            std::pair<float, float>{0, 0},
            new DyEnemy(GetScreenWidth(), 300)});

        count--;
        delay = 20;
    }
}

void Program::DrawStartup()
{
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, (float)GetScreenWidth(), (float)GetScreenHeight(), Color{0, 0, 0, 140});

    const Color soloColor = coop_selected ? GRAY : YELLOW;
    const Color coopColor = coop_selected ? YELLOW : GRAY;

    DrawText("Galaga", (screenWidth - MeasureText("Galaga", 144)) / 2, 140, 144, WHITE);
    DrawText("Select Mode", (screenWidth - MeasureText("Select Mode", 36)) / 2, 400, 36, WHITE);

    int gap = 60;
    int soloWidth = MeasureText("Solo", 32);
    int coopWidth = MeasureText("Co-op", 32);
    int TotalWidth = soloWidth + coopWidth + gap;
    int startX = (screenWidth - TotalWidth) / 2;

    DrawText("Solo", startX, 470, 32, soloColor);
    DrawText("Co-op", startX + soloWidth + gap, 470, 32, coopColor);

    DrawText("Press Enter to Start", (screenWidth - MeasureText("Press Enter to Start", 24)) / 2, 540, 24, WHITE);

    DrawText("P1: A / D to move, Space to shoot.", (screenWidth - MeasureText("P1: A / D to move, Space to shoot.", 24)) / 2, 620, 24, WHITE);
    DrawText("P2: Left / Right to move, right Shift to shoot.", (screenWidth - MeasureText("P2: Left / Right to move, right Shift to shoot.", 24)) / 2, 660, 24, WHITE);
    DrawText("Use Left/Right to choose, Enter to start", (screenWidth - MeasureText("Use Left/Right to choose, Enter to start", 24)) / 2, 710, 24, LIGHTGRAY);
}

void Program::DrawPauseScreen()
{
    DrawRectangle(0, 0, (float)GetScreenWidth(), (float)GetScreenHeight(), Color{0, 0, 0, 125});
    DrawText("Paused", (GetScreenWidth() / 2) - 85, GetScreenHeight() / 2 - 60, 48, WHITE);
    DrawText("Press Enter", (GetScreenWidth() / 2) - 75, GetScreenHeight() / 2, 24, GRAY);
}

void Program::DrawGameOver()
{
    DrawRectangle(0, 0, (float)GetScreenWidth(), (float)GetScreenHeight(), Color{0, 0, 0, 125});
    DrawText("Game Over", (GetScreenWidth() / 2) - 380, 50, 144, WHITE);
    DrawText("Press Enter", (GetScreenWidth() / 2) - 75, GetScreenHeight() / 2, 24, GRAY);
}

void Program::KeyInputs()
{
    if (startup)
    {
        if (IsKeyPressed(KEY_RIGHT))
            coop_selected = true;
        if (IsKeyPressed(KEY_LEFT))
            coop_selected = false;
        if (IsKeyPressed(KEY_ENTER))
        {
            coop_enabled = coop_selected;

            player->position.first = coop_enabled ? GetScreenWidth() / 2 - 60 : GetScreenWidth() / 2 - 15;
            player->position.second = GetScreenHeight() * 0.75f;

            if (coop_enabled)
            {
                player2 = new Player(GetScreenWidth() / 2 + 30, GetScreenHeight() * 0.75f);
            }
            else
            {
                player2 = nullptr;
            }

            startup = false;
        }

        return;
    }

    if ((!gameOver && IsKeyPressed('P')) || (paused && IsKeyPressed(KEY_ENTER)))
        paused = !paused;

    if (!paused && IsKeyPressed('O'))
        gameOver = !gameOver;

    if (IsKeyPressed('H'))
        HitBox::drawHitbox = !HitBox::drawHitbox;

    if (IsKeyPressed('K'))
        AddScore(500);

    if (gameOver && IsKeyPressed(KEY_ENTER))
    {
        gameOver = false;
        Reset();
    }

    if (!paused && !gameOver && pauseFrames <= 0)
    {
        player->keyInputs('A', 'D', KEY_SPACE);

        if (coop_enabled && player2 != nullptr)
            player2->keyInputs(KEY_LEFT, KEY_RIGHT, KEY_RIGHT_SHIFT);
    }
}

void Program::PlayerReset(Player *target)
{
    Animation::animations.push_back(
        Animation(target->position.first, target->position.second, 16, 0, 33, 34, 30, 30, 3, ImageManager::SpriteSheet));

    PlaySound(SoundManager::gameOver);
    Projectile::projectiles.clear();

    target->position.second = GetScreenHeight() * 0.75f;

    if (coop_enabled)
    {
        if (target == player)
            target->position.first = GetScreenWidth() / 2 - 60;
        else
            target->position.first = GetScreenWidth() / 2 + 30;
    } else {
        target->position.first = GetScreenWidth() / 2 - 15;
    }

    pauseFrames = 120;
    lives--;
}

void Program::Reset()
{
    Enemy::enemies.clear();
    Background::sideWalls = std::pair<HitBox, HitBox>{
        HitBox(0, 0, 10, GetScreenHeight()),
        HitBox(GetScreenWidth() - 10, 0, 10, GetScreenHeight())};

    Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy *>{
        std::pair<float, float>{350, 150},
        new SpEnemy(350, 150)});

    Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy *>{
        std::pair<float, float>{600, 150},
        new SpEnemy(600, 150)});

    for (int i = 0; i < 30; i++)
    {
        float x = 250 + 50 * (i % 10);
        float y = 200 + 50 * (i / 10);

        Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy *>{
            std::pair<float, float>{x, y},
            new StdEnemy(x, y)});
    }

    StdEnemy::attackInProgress = false;
    if (coop_enabled)
    {
        player = new Player((GetScreenWidth() / 2) - 60, GetScreenHeight() * 0.75f);
        player2 = new Player((GetScreenWidth() / 2) + 30, GetScreenHeight() * 0.75f);
    }
    else
    {
        player = new Player((GetScreenWidth() / 2) - 15, GetScreenHeight() * 0.75f);
        player2 = nullptr;
    }
    respawnCooldown = 1080;
    respawns = 0;
    count = 0;
    delay = 0;
    lives = 3;
    score = 0;
    nextLifeScore = 1000;
}
void Program::AddScore(int points)
{
    if (points <= 0)
        return;
    score += points;

    while (score >= nextLifeScore)
    {
        if (lives < maxLives)
            lives++;
        nextLifeScore += 1000;
    }
}

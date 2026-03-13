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
        HitBox enemyTarget = player->hitBox;
        if (!IsPlayerAlive(player) && coop_enabled && IsPlayerAlive(player2))
            enemyTarget = player2->hitBox;

        AddScore(Enemy::ManageEnemies(enemyTarget));
        StdEnemy::attackReset();
        ManageEnemyRespawns();

        if (IsPlayerAlive(player))
            player->update();
        if (coop_enabled && player2 != nullptr && IsPlayerAlive(player2))
            player2->update();

        for (std::pair<std::pair<float, float>, Enemy *> p : Enemy::enemies)
        {
            if (!p.second)
                continue;
            if (IsPlayerAlive(player) && HitBox::Collision(player->hitBox, p.second->hitBox))
            {
                p.second->health = 0;
                PlayerReset(player);
            }
            if (coop_enabled && player2 != nullptr && IsPlayerAlive(player2) && HitBox::Collision(player2->hitBox, p.second->hitBox))
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
                if (IsPlayerAlive(player) && HitBox::Collision(player->hitBox, p.getHitBox()))
                {
                    p.del = true;
                    PlayerReset(player);
                }
                if (coop_enabled && player2 != nullptr && IsPlayerAlive(player2) && HitBox::Collision(player2->hitBox, p.getHitBox()))
                {
                    p.del = true;
                    PlayerReset(player2);
                }
            }
        }

        if (livesP1 <= 0 && (!coop_enabled || livesP2 <= 0))
            gameOver = true;

            if (clearProjectilesNextFrame) {
                Projectile::projectiles.clear();
                clearProjectilesNextFrame = false;
            }

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
        if (IsPlayerAlive(player))
        {
            player->draw();

            if (coop_enabled)
                DrawText("P1",
                         player->position.first + (30 - MeasureText("P1", 25)) / 2,
                         player->position.second - 25, 25,
                         SKYBLUE);
        }

        if (coop_enabled && player2 != nullptr && IsPlayerAlive(player2))
        {
            player2->draw();
            DrawText("P2",
                     player2->position.first + (30 - MeasureText("P2", 25)) / 2,
                     player2->position.second - 25, 25,
                     PINK);
        }
    }
    for (Animation &a : Animation::animations)
        a.draw();

    DrawText("P1", 10, GetScreenHeight() - 55, 20, SKYBLUE);
    for (int i = 0; i < livesP1; i++)
    {
        DrawTexturePro(ImageManager::SpriteSheet, Rectangle{0, 0, 17, 18},
                       Rectangle{45.0f + i * 30, GetScreenHeight() - 53.0f, 20, 20},
                       Vector2{0, 0}, 0, WHITE);
    }

    if (coop_enabled)
    {
        int p2LivesStartX = GetScreenWidth() - 85 - (livesP2 - 1) * 30;
        int p2LabelX = p2LivesStartX - 35;

        DrawText("P2", p2LabelX, GetScreenHeight() - 55, 20, PINK);
        for (int i{}; i < livesP2; ++i)
        {
            DrawTexturePro(ImageManager::SpriteSheet, Rectangle{0, 0, 17, 18},
                           Rectangle{GetScreenWidth() - 85.0f - i * 30, GetScreenHeight() - 53.0f, 20, 20},
                           Vector2{0, 0}, 0, WHITE);
        }
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

    int baseRespawn = 620;
    int decay = 2 + (score / 1800);
    int decayCap = 4;
    int burstTrigger = 4;
    int burstCount = 3;
    int spawnDelayFrames = 22;

    if (activeDifficulty == 0)
    {
        baseRespawn = 900;
        decay = 1 + (score / 2600);
        decayCap = 2;
        burstTrigger = 4;
        burstCount = 2;
        spawnDelayFrames = 28;
    }
    else if (activeDifficulty == 2)
    {
        baseRespawn = 420;
        decay = 3 + (score / 1400);
        decayCap = 6;
        burstTrigger = 4;
        burstCount = 4;
        spawnDelayFrames = 18;
    }

    if (decay > decayCap)
        decay = decayCap;

    respawnCooldown = std::max(respawnCooldown - decay, 0);
    if (respawnCooldown <= 0)
    {
        respawnCooldown = baseRespawn;
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

    if (respawns >= burstTrigger)
    {
        count = burstCount;
        respawns = 0;
    }

    if (count > 0 && delay <= 0)
    {
        Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy *>{
            std::pair<float, float>{0, 0},
            new DyEnemy(GetScreenWidth(), 300)});

        count--;
        delay = spawnDelayFrames;
    }
}


void Program::DrawStartup()
{
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, (float)GetScreenWidth(), (float)GetScreenHeight(), Color{0, 0, 0, 140});

    const Color soloColor = coop_selected ? GRAY : YELLOW;
    const Color coopColor = coop_selected ? YELLOW : GRAY;

    const Color easyColor = selectedDifficulty == 0 ? YELLOW : GRAY;
    const Color mediumColor = selectedDifficulty == 1 ? YELLOW : GRAY;
    const Color hardColor = selectedDifficulty == 2 ? YELLOW : GRAY;

    DrawText("Galaga", (screenWidth - MeasureText("Galaga", 144)) / 2, 120, 144, WHITE); // btw follow this formatting for centering text
    DrawText("Select Mode", (screenWidth - MeasureText("Select Mode", 36)) / 2, 360, 36, WHITE);

    int mgap = 60;
    int soloWidth = MeasureText("Solo", 32);
    int coopWidth = MeasureText("Co-op", 32);
    int mTotalWidth = soloWidth + coopWidth + mgap;
    int mstartX = (screenWidth - mTotalWidth) / 2;

    DrawText("Solo", mstartX, 420, 32, soloColor);
    DrawText("Co-op", mstartX + soloWidth + mgap, 420, 32, coopColor);

    DrawText("Select Difficulty", (screenWidth - MeasureText("Select Difficulty", 36)) / 2, 500, 36, WHITE);

    int diffGFap = 60;
    int easyWidth = MeasureText("Easy", 28);
    int mediumWidth = MeasureText("Medium", 28);
    int hardWidth = MeasureText("Hard", 28);
    int diffTotalWidth = easyWidth + mediumWidth + hardWidth + 2 * diffGFap;
    int diffStartX = (screenWidth - diffTotalWidth) / 2;

    DrawText("Easy", diffStartX, 555, 28, easyColor);
    DrawText("Medium", diffStartX + easyWidth + diffGFap, 555, 28, mediumColor);
    DrawText("Hard", diffStartX + easyWidth + mediumWidth + 2 * diffGFap, 555, 28, hardColor);

    DrawText("Enter = Start", (screenWidth - MeasureText("Enter = Start", 24)) / 2, 625, 24, WHITE);
    DrawText("Left / Right = Co-op toggle", (screenWidth - MeasureText("Left / Right = Co-op toggle", 24)) / 2, 690, 24, WHITE);
    DrawText("Up / Down = Difficulty", (screenWidth - MeasureText("Up / Down = Difficulty", 24)) / 2, 725, 24, WHITE);

    DrawText("P1: A / D to move, Space to shoot", (screenWidth - MeasureText("P1: A / D to move, Space to shoot", 22)) / 2, 800, 22, WHITE);
    DrawText("P2: Left / Right to move, Right Shift to shoot", (screenWidth - MeasureText("P2: Left / Right to move, Right Shift to shoot", 22)) / 2, 835, 22, WHITE);
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
        if (IsKeyPressed(KEY_UP) && selectedDifficulty > 0)
            selectedDifficulty--;
        if (IsKeyPressed(KEY_DOWN) && selectedDifficulty < 2)
            selectedDifficulty++;

        if (IsKeyPressed(KEY_ENTER))
        {
            coop_enabled = coop_selected;
            activeDifficulty = selectedDifficulty;
            Enemy::difficultyLevel = activeDifficulty;

            livesP1 = 3;
            livesP2 = coop_enabled ? 3 : 0;

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
        if (IsPlayerAlive(player))
            player->keyInputs('A', 'D', KEY_SPACE);

        if (coop_enabled && player2 != nullptr && IsPlayerAlive(player2))
            player2->keyInputs(KEY_LEFT, KEY_RIGHT, KEY_RIGHT_SHIFT);
    }
}


bool Program::IsPlayerAlive(Player *target)
{
    if (target == player)
        return livesP1 > 0;

    if (target == player2)
        return coop_enabled && player2 != nullptr && livesP2 > 0;

    return false;
}

void Program::PlayerReset(Player *target)
{
    Animation::animations.push_back(
        Animation(target->position.first, target->position.second, 16, 0, 33, 34, 30, 30, 3, ImageManager::SpriteSheet));

    PlaySound(SoundManager::gameOver);
    clearProjectilesNextFrame = true;

    if (target == player)
        livesP1--;
    else if (target == player2)
        livesP2--;

    if (!IsPlayerAlive(target))
    {
        target->position.first = -100;
        target->position.second = -100;
        target->hitBox.box.x = -100;
        target->hitBox.box.y = -100;
        pauseFrames = 120;
        return;
    }

    target->position.second = GetScreenHeight() * 0.75f;

    if (coop_enabled)
    {
        if (target == player)
            target->position.first = GetScreenWidth() / 2 - 60;
        else
            target->position.first = GetScreenWidth() / 2 + 30;
    }
    else
    {
        target->position.first = GetScreenWidth() / 2 - 15;
    }

    pauseFrames = 120;
}


void Program::Reset()
{
    Enemy::enemies.clear();
    Background::sideWalls = std::pair<HitBox, HitBox>{
        HitBox(0, 0, 10, GetScreenHeight()),
        HitBox(GetScreenWidth() - 10, 0, 10, GetScreenHeight())};

    Enemy::difficultyLevel = activeDifficulty;

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

    livesP1 = 3;
    livesP2 = coop_enabled ? 3 : 0;

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
    respawnCooldown = (activeDifficulty == 0) ? 900 : (activeDifficulty == 2 ? 420 : 620);
    respawns = 0;
    count = 0;
    delay = 0;
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
        if (livesP1 < maxLives)
            livesP1++;

        if (coop_enabled && livesP2 > 0 && livesP2 < maxLives)
            livesP2++;

        nextLifeScore += 1000;
    }
}


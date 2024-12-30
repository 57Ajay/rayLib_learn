#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_OBSTACLES 20
#define INITIAL_OBSTACLES 2
#define HIGHSCORE_FILE "highscore.dat"
#define MAX_POWERUPS 5

typedef struct {
  Rectangle rect;
  Vector2 speed;
  Color color;
  bool active;
} GameObject;

typedef struct {
  Rectangle rect;
  Texture2D texture;
  bool active;
  float timer;
  float duration;
  int type; // Ex -> 0: Invincibility
} PowerUp;

typedef struct {
  Vector2 position;
  Color color;
  float radius;
  float alpha;
  bool active;
} FloorHitEffect;

// spawn a new obstacle with relative positioning
void SpawnObstacle(GameObject *obstacle, float screenWidth, float baseSpeed,
                   float relativeX) {
  obstacle->rect.x = relativeX * screenWidth;
  obstacle->rect.y = -obstacle->rect.height;
  obstacle->speed.y = baseSpeed + GetRandomValue(0, 200) / 100.0f;
  obstacle->active = true;

  Color colors[] = {RED, DARKGRAY, MAROON, ORANGE, DARKGREEN};
  obstacle->color = colors[GetRandomValue(0, 4)];
}

// spawn a new power-up
void SpawnPowerUp(PowerUp *powerUp, float screenWidth, float screenHeight) {
  powerUp->rect.width = 30;
  powerUp->rect.height = 30;
  powerUp->rect.x = GetRandomValue(50, screenWidth - 50 - powerUp->rect.width);
  powerUp->rect.y =
      GetRandomValue(50, screenHeight - 200 - powerUp->rect.height);
  powerUp->active = true;
  powerUp->timer = 0.0f;
  powerUp->duration = 5.0f; // Example duration
  powerUp->type = 0;        // Invincibility
}

int LoadHighScore() {
  int highScore = 0;
  FILE *file = fopen(HIGHSCORE_FILE, "rb");
  if (file) {
    fread(&highScore, sizeof(int), 1, file);
    fclose(file);
  }
  return highScore;
}

void SaveHighScore(int score) {
  FILE *file = fopen(HIGHSCORE_FILE, "wb");
  if (file) {
    fwrite(&score, sizeof(int), 1, file);
    fclose(file);
  }
}

int main(void) {
  int screenWidth = 800;
  int screenHeight = 600;
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidth, screenHeight, "Dynamic Dodge Game");
  SetWindowMinSize(400, 300); // sets minimum window size
  SetRandomSeed(time(NULL));
  InitAudioDevice();

  // sounds
  Sound collisionSound = LoadSound("resources/collision.wav");
  if (collisionSound.frameCount == 0)
    printf("Error loading collision.wav\n");
  Sound powerUpSound = LoadSound("resources/powerup.wav");
  if (powerUpSound.frameCount == 0)
    printf("Error loading powerup.wav\n");
  Sound floorHitSound = LoadSound("resources/floorhit.wav");
  if (floorHitSound.frameCount == 0)
    printf("Error loading floorhit.wav\n");

  GameObject player = {
      .rect = {0.5f * screenWidth - 15, 0.8f * screenHeight, 30, 30},
      .speed = {5.0f, 5.0f},
      .color = BLUE,
      .active = true};

  // obstacle array
  GameObject obstacles[MAX_OBSTACLES];
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].rect = (Rectangle){0, 0, 30, 30};
    obstacles[i].active = false;
  }

  // Initializes power-up array
  PowerUp powerUps[MAX_POWERUPS];
  Texture2D invincibilityTexture = LoadTexture("resources/star.png");
  if (invincibilityTexture.id == 0)
    printf("Error loading star.png\n");
  for (int i = 0; i < MAX_POWERUPS; i++) {
    powerUps[i].rect = (Rectangle){0, 0, 30, 30};
    powerUps[i].active = false;
    powerUps[i].texture = invincibilityTexture;
  }

  // Initializes floor hit effects
  FloorHitEffect floorHits[MAX_OBSTACLES];
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    floorHits[i].active = false;
  }

  int score = 0;
  int highScore = LoadHighScore();
  int activeObstacles = INITIAL_OBSTACLES;
  float baseSpeed = 3.0f;
  bool gameOver = false;
  bool gamePaused = false;
  float timePlayed = 0.0f;
  int nextObstacleScore = 500;
  bool isInvincible = false;
  float invincibilityTimer = 0.0f;
  float invincibilityDuration = 5.0f;
  float powerUpSpawnTimer = 0.0f;
  float powerUpSpawnInterval = 10.0f;

  for (int i = 0; i < INITIAL_OBSTACLES; i++) {
    float relativeX = GetRandomValue(0, 100) / 100.0f;
    SpawnObstacle(&obstacles[i], screenWidth, baseSpeed, relativeX);
  }

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    float deltaTime = GetFrameTime();

    if (IsWindowResized()) {
      screenWidth = GetScreenWidth();
      screenHeight = GetScreenHeight();
      // Adjust player position relative to new screen size
      player.rect.y = 0.8f * screenHeight;
    }

    if (IsKeyPressed(KEY_SPACE))
      gamePaused = !gamePaused;

    if (!gameOver && !gamePaused) {
      // Update time and score
      timePlayed += deltaTime;
      score = (int)(timePlayed * 100);

      // Update high score
      if (score > highScore) {
        highScore = score;
        SaveHighScore(highScore);
      }

      // Increase the difficulty based on score
      baseSpeed = 3.0f + (score / 1000.0f);

      // Add a new obstacle when score threshold is reached
      if (score >= nextObstacleScore && activeObstacles < MAX_OBSTACLES) {
        activeObstacles++;
        nextObstacleScore += 500;
        for (int i = 0; i < MAX_OBSTACLES; i++) {
          if (!obstacles[i].active) {
            float relativeX = GetRandomValue(0, 100) / 100.0f;
            SpawnObstacle(&obstacles[i], screenWidth, baseSpeed, relativeX);
            break;
          }
        }
      }

      // Power-up the spawning
      powerUpSpawnTimer += deltaTime;
      if (powerUpSpawnTimer >= powerUpSpawnInterval) {
        powerUpSpawnTimer = 0.0f;
        for (int i = 0; i < MAX_POWERUPS; i++) {
          if (!powerUps[i].active) {
            SpawnPowerUp(&powerUps[i], screenWidth, screenHeight);
            break;
          }
        }
      }

      float moveSpeed = player.speed.x * (screenWidth / 800.0f);
      if (IsKeyDown(KEY_RIGHT) &&
          (player.rect.x + player.rect.width) < screenWidth)
        player.rect.x += moveSpeed;
      if (IsKeyDown(KEY_LEFT) && player.rect.x > 0)
        player.rect.x -= moveSpeed;
      if (IsKeyDown(KEY_UP) && player.rect.y > 0)
        player.rect.y -= moveSpeed;
      if (IsKeyDown(KEY_DOWN) &&
          (player.rect.y + player.rect.height) < screenHeight)
        player.rect.y += moveSpeed;

      // Update power-ups
      for (int i = 0; i < MAX_POWERUPS; i++) {
        if (powerUps[i].active) {
          if (CheckCollisionRecs(player.rect, powerUps[i].rect)) {
            PlaySound(powerUpSound);
            powerUps[i].active = false;
            isInvincible = true;
            invincibilityTimer = 0.0f;
          }
        }
      }

      // Handling invincibility
      if (isInvincible) {
        invincibilityTimer += deltaTime;
        if (invincibilityTimer >= invincibilityDuration) {
          isInvincible = false;
        }
      }

      // Updating the obstacles and handling the collisions
      for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
          obstacles[i].rect.y += obstacles[i].speed.y;

          if (obstacles[i].rect.y > screenHeight) {
            // floor hit effect triggered
            for (int j = 0; j < MAX_OBSTACLES; j++) {
              if (!floorHits[j].active) {
                floorHits[j].active = true;
                floorHits[j].position =
                    (Vector2){obstacles[i].rect.x + obstacles[i].rect.width / 2,
                              screenHeight};
                floorHits[j].color = obstacles[i].color;
                floorHits[j].radius = 10.0f;
                floorHits[j].alpha = 1.0f;
                PlaySound(floorHitSound);
                break;
              }
            }
            float relativeX = GetRandomValue(0, 100) / 100.0f;
            SpawnObstacle(&obstacles[i], screenWidth, baseSpeed, relativeX);
          }

          if (!isInvincible &&
              CheckCollisionRecs(player.rect, obstacles[i].rect)) {
            gameOver = true;
            PlaySound(collisionSound);
            break;
          }
        }
      }

      // Updates the floor hit effects
      for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (floorHits[i].active) {
          floorHits[i].radius += 5.0f;
          floorHits[i].alpha -= 0.05f;
          if (floorHits[i].alpha <= 0.0f) {
            floorHits[i].active = false;
          }
        }
      }
    }

    BeginDrawing();
    {
      ClearBackground(RAYWHITE);

      if (!gameOver) {
        // Drawing the player (with invincibility color)
        DrawRectangleRec(player.rect, isInvincible ? YELLOW : player.color);

        // Drawing obstacles
        for (int i = 0; i < MAX_OBSTACLES; i++) {
          if (obstacles[i].active) {
            DrawRectangleRec(obstacles[i].rect, obstacles[i].color);
          }
        }

        // Drawing power-ups
        for (int i = 0; i < MAX_POWERUPS; i++) {
          if (powerUps[i].active) {
            DrawTexturePro(powerUps[i].texture,
                           (Rectangle){0, 0, powerUps[i].texture.width,
                                       powerUps[i].texture.height},
                           powerUps[i].rect, (Vector2){0, 0}, 0.0f, WHITE);
          }
        }

        // Drawing floor hit effects
        for (int i = 0; i < MAX_OBSTACLES; i++) {
          if (floorHits[i].active) {
            DrawCircleV(floorHits[i].position, floorHits[i].radius,
                        Fade(floorHits[i].color, floorHits[i].alpha));
          }
        }

        // Draw HUD
        DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
        DrawText(TextFormat("High Score: %d", highScore), 10, 40, 20, BLACK);
        DrawText(TextFormat("Speed: %.1f", baseSpeed), 10, 70, 20, BLACK);
        DrawText(TextFormat("Obstacles: %d", activeObstacles), 10, 100, 20,
                 BLACK);
        if (isInvincible) {
          DrawText(TextFormat("Invincible: %.1f",
                              invincibilityDuration - invincibilityTimer),
                   10, 130, 20, GOLD);
        }

        // Drawing pause message
        if (gamePaused) {
          DrawText("PAUSED", screenWidth / 2 - 60, screenHeight / 2, 40, GRAY);
          DrawText("Press SPACE to continue", screenWidth / 2 - 120,
                   screenHeight / 2 + 50, 20, DARKGRAY);
        }

      } else {
        // Game over screen
        DrawText("Game Over!", screenWidth / 2 - 100, screenHeight / 2 - 50, 40,
                 RED);
        DrawText(TextFormat("Score: %d", score), screenWidth / 2 - 70,
                 screenHeight / 2, 30, BLACK);
        DrawText(TextFormat("High Score: %d", highScore), screenWidth / 2 - 90,
                 screenHeight / 2 + 40, 30, GOLD);
        DrawText("Press R to Restart", screenWidth / 2 - 100,
                 screenHeight / 2 + 80, 20, DARKGRAY);
      }
    }
    EndDrawing();

    // Reset the game
    if (gameOver && IsKeyPressed(KEY_R)) {
      // Reset player to relative position
      player.rect.x = 0.5f * screenWidth - 15;
      player.rect.y = 0.8f * screenHeight;
      isInvincible = false;

      // Reset the obstacles
      activeObstacles = INITIAL_OBSTACLES;
      for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = false;
      }
      for (int i = 0; i < INITIAL_OBSTACLES; i++) {
        float relativeX = GetRandomValue(0, 100) / 100.0f;
        SpawnObstacle(&obstacles[i], screenWidth, baseSpeed, relativeX);
      }

      // Reset power-ups
      for (int i = 0; i < MAX_POWERUPS; i++) {
        powerUps[i].active = false;
      }
      powerUpSpawnTimer = 0.0f;

      // Reset the floor hit effects
      for (int i = 0; i < MAX_OBSTACLES; i++) {
        floorHits[i].active = false;
      }

      // Reset game state
      score = 0;
      timePlayed = 0.0f;
      baseSpeed = 3.0f;
      nextObstacleScore = 500;
      gameOver = false;
      gamePaused = false;
    }
  }

  UnloadTexture(invincibilityTexture);

  UnloadSound(collisionSound);
  UnloadSound(powerUpSound);
  UnloadSound(floorHitSound);

  CloseAudioDevice();
  CloseWindow();
  return 0;
}

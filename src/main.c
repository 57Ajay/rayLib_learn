#include "raylib.h"
#include <stdio.h>
// #include <stdlib.h>

#define MAX_OBSTACLES 20
#define INITIAL_OBSTACLES 2
#define HIGHSCORE_FILE "highscore.dat"

typedef struct {
  Rectangle rect;
  Vector2 speed;
  Color color;
  bool active;
} GameObject;

// Function to spawn a new obstacle with relative positioning
void SpawnObstacle(GameObject *obstacle, float screenWidth, float baseSpeed,
                   float relativeX) {
  obstacle->rect.x = relativeX * screenWidth;
  obstacle->rect.y = -obstacle->rect.height;
  obstacle->speed.y = baseSpeed + GetRandomValue(0, 200) / 100.0f;
  obstacle->active = true;

  Color colors[] = {RED, DARKGRAY, MAROON, ORANGE, DARKGREEN};
  obstacle->color = colors[GetRandomValue(0, 4)];
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
  SetWindowMinSize(400, 300); // Set minimum window size

  GameObject player = {
      .rect = {0.5f * screenWidth - 15, 0.8f * screenHeight, 30, 30},
      .speed = {5.0f, 5.0f},
      .color = BLUE,
      .active = true};

  // Initialize obstacle array
  GameObject obstacles[MAX_OBSTACLES];
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].rect = (Rectangle){0, 0, 30, 30};
    obstacles[i].active = false;
  }

  int score = 0;
  int highScore = LoadHighScore();
  int activeObstacles = INITIAL_OBSTACLES;
  float baseSpeed = 3.0f;
  bool gameOver = false;
  bool gamePaused = false;
  float timePlayed = 0.0f;
  int nextObstacleScore = 500;

  for (int i = 0; i < INITIAL_OBSTACLES; i++) {
    float relativeX = GetRandomValue(0, 100) / 100.0f;
    SpawnObstacle(&obstacles[i], screenWidth, baseSpeed, relativeX);
  }

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
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
      timePlayed += GetFrameTime();
      score = (int)(timePlayed * 100);

      // Update high score
      if (score > highScore) {
        highScore = score;
        SaveHighScore(highScore);
      }

      // Increase difficulty based on score
      baseSpeed = 3.0f + (score / 1000.0f);

      // Add new obstacle when score threshold is reached
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

      // Update obstacles
      for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
          obstacles[i].rect.y += obstacles[i].speed.y;

          if (obstacles[i].rect.y > screenHeight) {
            float relativeX = GetRandomValue(0, 100) / 100.0f;
            SpawnObstacle(&obstacles[i], screenWidth, baseSpeed, relativeX);
          }

          if (CheckCollisionRecs(player.rect, obstacles[i].rect)) {
            gameOver = true;
            break;
          }
        }
      }
    }

    BeginDrawing();
    {
      ClearBackground(RAYWHITE);

      if (!gameOver) {
        DrawRectangleRec(player.rect, player.color);
        for (int i = 0; i < MAX_OBSTACLES; i++) {
          if (obstacles[i].active) {
            DrawRectangleRec(obstacles[i].rect, obstacles[i].color);
          }
        }

        // Draw HUD
        DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
        DrawText(TextFormat("High Score: %d", highScore), 10, 40, 20, BLACK);
        DrawText(TextFormat("Speed: %.1f", baseSpeed), 10, 70, 20, BLACK);
        DrawText(TextFormat("Obstacles: %d", activeObstacles), 10, 100, 20,
                 BLACK);

        // Draw pause message
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

    // Reset game
    if (gameOver && IsKeyPressed(KEY_R)) {
      // Reset player to relative position
      player.rect.x = 0.5f * screenWidth - 15;
      player.rect.y = 0.8f * screenHeight;

      // Reset obstacles
      activeObstacles = INITIAL_OBSTACLES;
      for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = false;
      }
      for (int i = 0; i < INITIAL_OBSTACLES; i++) {
        float relativeX = GetRandomValue(0, 100) / 100.0f;
        SpawnObstacle(&obstacles[i], screenWidth, baseSpeed, relativeX);
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

  CloseWindow();
  return 0;
}

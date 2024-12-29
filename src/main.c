#include "raylib.h"

typedef struct {
  Rectangle rect;
  Vector2 speed;
  Color color;
} GameObject;

int main(void) {
  // Initialize window
  const int screenWidth = 800;
  const int screenHeight = 600;
  InitWindow(screenWidth, screenHeight, "Dodge Game");

  // Initialize player
  GameObject player = {
      .rect = {(float)screenWidth / 2 - 25, screenHeight - 100, 50, 50},
      .speed = {5.0f, 5.0f},
      .color = BLUE};

  // Initialize obstacles
  GameObject obstacle1 = {
      .rect = {100, 0, 40, 40}, .speed = {0.0f, 4.0f}, .color = RED};

  GameObject obstacle2 = {
      .rect = {400, 0, 40, 40}, .speed = {0.0f, 6.0f}, .color = DARKGRAY};

  // Game variables
  int score = 0;
  bool gameOver = false;
  float timePlayed = 0.0f;

  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose()) {
    if (!gameOver) {
      // Update time and score
      timePlayed += GetFrameTime();
      score = (int)timePlayed;

      // Player movement
      if (IsKeyDown(KEY_RIGHT) &&
          (player.rect.x + player.rect.width) < screenWidth)
        player.rect.x += player.speed.x;
      if (IsKeyDown(KEY_LEFT) && player.rect.x > 0)
        player.rect.x -= player.speed.x;
      if (IsKeyDown(KEY_UP) && player.rect.y > 0)
        player.rect.y -= player.speed.y;
      if (IsKeyDown(KEY_DOWN) &&
          (player.rect.y + player.rect.height) < screenHeight)
        player.rect.y += player.speed.y;

      // Update obstacles
      obstacle1.rect.y += obstacle1.speed.y;
      if (obstacle1.rect.y > screenHeight) {
        obstacle1.rect.y = -obstacle1.rect.height;
        obstacle1.rect.x =
            GetRandomValue(0, screenWidth - obstacle1.rect.width);
      }

      obstacle2.rect.y += obstacle2.speed.y;
      if (obstacle2.rect.y > screenHeight) {
        obstacle2.rect.y = -obstacle2.rect.height;
        obstacle2.rect.x =
            GetRandomValue(0, screenWidth - obstacle2.rect.width);
      }

      // Check collisions
      if (CheckCollisionRecs(player.rect, obstacle1.rect) ||
          CheckCollisionRecs(player.rect, obstacle2.rect)) {
        gameOver = true;
      }
    }

    // Draw
    BeginDrawing();
    {
      ClearBackground(RAYWHITE);

      if (!gameOver) {
        // Draw game objects
        DrawRectangleRec(player.rect, player.color);
        DrawRectangleRec(obstacle1.rect, obstacle1.color);
        DrawRectangleRec(obstacle2.rect, obstacle2.color);

        // Draw score
        DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);

      } else {
        // Draw game over screen
        DrawText("Game Over!", screenWidth / 2 - 100, screenHeight / 2 - 50, 40,
                 RED);
        DrawText(TextFormat("Final Score: %d", score), screenWidth / 2 - 100,
                 screenHeight / 2, 30, BLACK);
        DrawText("Press R to Restart", screenWidth / 2 - 100,
                 screenHeight / 2 + 50, 20, DARKGRAY);
      }
    }
    EndDrawing();

    // Reset game
    if (gameOver && IsKeyPressed(KEY_R)) {
      // Reset player
      player.rect.x = (float)screenWidth / 2 - 25;
      player.rect.y = screenHeight - 100;

      // Reset obstacles
      obstacle1.rect.y = 0;
      obstacle1.rect.x = 100;

      obstacle2.rect.y = 0;
      obstacle2.rect.x = 400;

      // Reset game state
      score = 0;
      timePlayed = 0.0f;
      gameOver = false;
    }
  }

  CloseWindow();
  return 0;
}

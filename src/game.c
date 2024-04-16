#include <raylib.h>
#include "sound.h"
#include "game.h"
#include "ball.h"
#include "objective.h"
#include "player.h"

static GameState sCurrentGameState;

void RunGame() {
    switch (sCurrentGameState) {
        case GAME_STATE_PLAYING: {
            // update
            float deltaTime = GetFrameTime();
            UpdatePlayer(deltaTime);
            UpdateBalls(deltaTime);
            UpdateObjectives(deltaTime);

            // render
            BeginDrawing();
            ClearBackground(BLACK);
            RenderObjectives();
            RenderBalls();
            RenderPlayer();
            EndDrawing();
            break;
        }

        case GAME_STATE_OVER:
            // update
            if (IsKeyPressed(KEY_ENTER)) {
                PlaySound(gRestartSound);
                ChangeGameStateTo(GAME_STATE_PLAYING);
            }

            // render
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("GAME OVER", GAME_WIDTH / 2, GAME_HEIGHT / 2, 40, WHITE);
            DrawText("press enter to restart", GAME_WIDTH / 2, (GAME_HEIGHT / 2) + 40, 20, WHITE);
            EndDrawing();
            break;
    }
}

void ChangeGameStateTo(GameState newState) {
    sCurrentGameState = newState;

    switch (sCurrentGameState) {
        case GAME_STATE_PLAYING:
            InitPlayer();
            InitObjectives();
            InitBalls();
            SpawnBall();
            break;
        case GAME_STATE_OVER:
            PlaySound(gRestartSound);
            break;
    }
}
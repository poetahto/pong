#include <raylib.h>
#include "sound.h"
#include "game.h"
#include "ball.h"
#include "objective.h"
#include "player.h"

static GameState sCurrentGameState;

void RunGame() {
    switch (sCurrentGameState) {
        case GamePlaying: {
            // update
            float deltaTime = GetFrameTime();
            UpdatePlayer(deltaTime);
            UpdateBalls(deltaTime);
            UpdateObjectives(deltaTime);

            // render
            BeginDrawing();
            ClearBackground(BLACK);
            RenderPlayer();
            RenderBalls();
            RenderObjectives();
            EndDrawing();
            break;
        }

        case GameOver:
            if (IsKeyPressed(KEY_ENTER)) {
                PlaySound(gRestartSound);
                ChangeGameStateTo(GamePlaying);
            }

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

    switch (sCurrentGameState) { // enter logic
        case GamePlaying: {
            InitPlayer();
            InitObjectives();
            InitBalls();
            break;
        }
        case GameOver: {
            PlaySound(gRestartSound);
            break;
        }
    }
}
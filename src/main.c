// Random small game project w/ raylib
// - Daniel Walls, 3/19/2024

// todo: ideas
// - music
// - background particles
// - particles on bounce hit (free list impl?)
// - multiple balls of diff color and diff colored effects sizes
// - ball leaves a trail that you must dodge
// - start to split things into different files...
// - data-oriented approach rather than hard code defines?

// - turn this into a touhou style boss fight?
//  - text box w/ dialogue about the beauty of flow, only when ur near death are you truly alive, wasting time
//  - health bar
//  - stages + attack patterns

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <incbin.h>
#include "sound.h"
#include "ball.h"
#include "player.h"
#include "objective.h"
#include "game.h"

typedef enum GameState {
    GamePlaying,
    GameOver
} GameState;

typedef struct BounceEffect {
    float remainingTime;
    Vector2 position;
} BounceEffect;

static BounceEffect sBounceEffects[MAX_BOUNCE_EFFECTS];
static float sTimeSinceBounce;

static Vector2 sBallVelocity;
static Vector2 sBallPosition;

static Vector2 sPlayerPosition;
static Vector2 sPlayerVelocity;
static Vector2 sPlayerSize;

static GameState sCurrentGameState;

static int Max(int a, int b);
static void HandleBounce(Vector2 position);
static void ChangeGameStateTo(GameState newState);

int main(void) {
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "PONG");
    InitAudioDevice();
    LoadSounds();

    ChangeGameStateTo(GamePlaying);

    while (!WindowShouldClose()) {
        switch (sCurrentGameState) {
            case GamePlaying: {
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText(TextFormat("%u collected", gCollectedObjectives),
                         190, 200, 20, WHITE);
                DrawText(TextFormat("%u highscore", gHighScoreObjectives),
                         190, 180, 20, WHITE);
                float deltaTime = GetFrameTime();

                // update player

                Vector2 inputDirection = Vector2Zero();

                if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
                    inputDirection.y -= 1;
                }
                if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
                    inputDirection.y += 1;
                }
                if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
                    inputDirection.x -= 1;
                }
                if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
                    inputDirection.x += 1;
                }

                inputDirection = Vector2Normalize(inputDirection);
                bool isAccelerating = (inputDirection.x != 0) || (inputDirection.y != 0);

                if (isAccelerating) {
                    Vector2 targetVelocity = Vector2Scale(inputDirection, PLAYER_SPEED);
                    sPlayerVelocity = Vector2MoveTowards(
                        sPlayerVelocity, targetVelocity, PLAYER_ACCELERATION * deltaTime);
                }
                else { // is decelerating
                    sPlayerVelocity = Vector2Lerp(
                            sPlayerVelocity, Vector2Zero(), PLAYER_DECELERATION * deltaTime);
                }

                sPlayerPosition = Vector2Add(
                    sPlayerPosition, Vector2Scale(sPlayerVelocity, deltaTime));

                sPlayerPosition.x = Clamp(
                    sPlayerPosition.x, 
                    PLAYER_WIDTH * 0.5f, 
                    GAME_WIDTH - PLAYER_WIDTH * 0.5f
                );
                sPlayerPosition.y = Clamp(
                    sPlayerPosition.y, 
                    PLAYER_HEIGHT * 0.5f, 
                    GAME_HEIGHT - PLAYER_HEIGHT * 0.5f
                );

                // animate size based on speed
                sPlayerSize.y = Lerp(
                    PLAYER_WIDTH,
                    PLAYER_WIDTH * PLAYER_SQUISH_AMOUNT,
                    fabsf(sPlayerVelocity.x) / PLAYER_SPEED);
                sPlayerSize.x = Lerp(
                    PLAYER_HEIGHT,
                    PLAYER_HEIGHT * PLAYER_SQUISH_AMOUNT,
                    fabsf(sPlayerVelocity.y) / PLAYER_SPEED);

                // update bounce effects

                for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
                    // skip if not ObjectivesActive
                    if (sBounceEffects[i].remainingTime <= 0) {
                        continue;
                    }
                    sBounceEffects[i].remainingTime -= deltaTime;

                    // find the percent complete we are with the effect
                    float t = sBounceEffects[i].remainingTime / BOUNCE_EFFECT_DURATION;

                    // the size multiplier should start at 1 and end at BOUNCE_EFFECT_MAX_SIZE
                    float bounceSizeMultiplier = 
                        1 + ((BOUNCE_EFFECT_MAX_SIZE - 1) * (1 - t));

                    // the color should fade out as the effect completes
                    Color color = WHITE;
                    color.a = (unsigned char)((float) color.a * t);

                    // render the bounce effect
                    float outerRadius = BALL_SIZE * bounceSizeMultiplier;
                    float innerRadius = fmaxf(outerRadius - BOUNCE_EFFECT_WIDTH, 0);
                    DrawRing(sBounceEffects[i].position, innerRadius, outerRadius, 
                             0, 360, 30, color);
                }

                // update ball

                sTimeSinceBounce += deltaTime;

                // how close are we to going max-speed?
                float velocityPercent = Clamp(sTimeSinceBounce / BALL_ACCELERATION_TIME, 0, 1);
                // update velocity based on acceleration rate
                Vector2 initialVelocity = Vector2Scale(
                    Vector2Normalize(sBallVelocity), BALL_SPEED);
                Vector2 targetVelocity = Vector2Scale(
                    Vector2Normalize(sBallVelocity), BALL_MAX_SPEED);
                sBallVelocity = Vector2Lerp(initialVelocity, targetVelocity, velocityPercent);

                // update position based on velocity for this frame
                sBallPosition = Vector2Add(
                    Vector2Scale(sBallVelocity, deltaTime), sBallPosition);

                float curBallSize = Lerp(BALL_SIZE, BALL_MIN_SIZE, velocityPercent);

                // bounce off-screen top/bottom
                if (sBallPosition.x > GAME_WIDTH || sBallPosition.x < 0) {
                    sBallVelocity.x = -sBallVelocity.x;
                    sBallPosition.x = Clamp(sBallPosition.x, 0, GAME_WIDTH);
                    HandleBounce(sBallPosition);
                }

                // bounce off-screen left/right
                if (sBallPosition.y > GAME_HEIGHT || sBallPosition.y < 0) {
                    sBallVelocity.y = -sBallVelocity.y;
                    sBallPosition.y = Clamp(sBallPosition.y, 0, GAME_HEIGHT);
                    HandleBounce(sBallPosition);
                }


                // render ball
                DrawCircleV(sBallPosition, curBallSize, WHITE);

                // render player
                Vector2 renderedPos = sPlayerPosition;
                renderedPos.x -= 0.5f * sPlayerSize.x;
                renderedPos.y -= 0.5f * sPlayerSize.y;
                DrawRectangleV(renderedPos, sPlayerSize, WHITE);

                // check collision
                Rectangle playerRect = {
                    .x = renderedPos.x,
                    .y = renderedPos.y,
                    .width = sPlayerSize.x,
                    .height = sPlayerSize.y,
                };

                // between player and the ball
                if (CheckCollisionCircleRec(sBallPosition, curBallSize, playerRect)) {
                    ChangeGameStateTo(GameOver);
                }

                UpdateObjectives(deltaTime, playerRect);

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

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

static void HandleBounce(Vector2 position) {
    // find the first unused effect
    int effectId = -1;

    for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
        if (sBounceEffects[i].remainingTime <= 0) {
            effectId = i;
            break;
        }
    }

    if (effectId == -1) {
        printf("ran out of bounce effects!\n");
        return;
    }

    PlaySound(gBallHitSound);

    // initialize new bounce effect
    sBounceEffects[effectId].remainingTime = BOUNCE_EFFECT_DURATION;
    sBounceEffects[effectId].position = position;

    // reset ball speed + acceleration
    sTimeSinceBounce = 0;
    sBallVelocity = Vector2Scale(Vector2Normalize(sBallVelocity), BALL_SPEED);
}

static void ChangeGameStateTo(GameState newState) {
    sCurrentGameState = newState;

    switch (sCurrentGameState) { // enter logic
        case GamePlaying: {
            sBallVelocity.x = BALL_SPEED;
            sBallVelocity.y = BALL_SPEED;

            sBallPosition.x = GAME_WIDTH / 2.0f;
            sBallPosition.y = GAME_HEIGHT / 2.0f;

            sPlayerPosition.x = GAME_WIDTH / 2.0f;
            sPlayerPosition.y = (GAME_HEIGHT / 2.0f) + 100;

            sPlayerVelocity = Vector2Zero();

            sPlayerSize.x = PLAYER_WIDTH;
            sPlayerSize.y = PLAYER_HEIGHT;

            for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
                sBounceEffects[i].remainingTime = 0;
            }

            ChangeObjectiveStateTo(ObjectivesInitialized);
            break;
        }
        case GameOver: {
            PlaySound(gRestartSound);
            break;
        }
    }
}

static int Max(int a, int b) {
    return a > b ? a : b;
}

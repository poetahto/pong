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
#include <stdlib.h>
#include <incbin.h>

#define GAME_WIDTH 800
#define GAME_HEIGHT 600

#define OBJECTIVE_GROUP_SIZE 3
#define OBJECTIVE_SIZE 40        // in pixels
#define OBJECTIVE_ANIM_TIME 15   // in weight lerp units
#define OBJECTIVE_DELAY_TIME 1   // in seconds
#define OBJECTIVE_ROTATE_SPEED 2 // in degrees per second

#define BALL_SIZE 35                // in pixels
#define BALL_MIN_SIZE 6             // in pixels
#define BALL_SPEED 10               // in pixels per second
#define BALL_MAX_SPEED 2000         // in pixels per second
#define BALL_ACCELERATION_TIME 0.5f // in seconds

#define MAX_BOUNCE_EFFECTS 16
#define BOUNCE_EFFECT_DURATION 1.5f // in seconds
#define BOUNCE_EFFECT_MAX_SIZE 5    // multiple of original size
#define BOUNCE_EFFECT_WIDTH 2       // in pixels

#define PLAYER_WIDTH 25           // in pixels
#define PLAYER_HEIGHT 28          // in pixels
#define PLAYER_SPEED 500          // in pixels per second
#define PLAYER_ACCELERATION 4000  // in pixels per second per second
#define PLAYER_DECELERATION 15   // in weird lerp units LOL
#define PLAYER_SQUISH_AMOUNT 0.4f // in percent size

typedef enum GameState {
    GamePlaying,
    GameOver
} GameState;

typedef enum ObjectiveState {
    ObjectivesActive,
    ObjectivesDelayed
} ObjectiveState;

typedef struct BounceEffect {
    float remainingTime;
    Vector2 position;
} BounceEffect;

typedef struct Objective {
    Vector2 position;
    float size;
    bool isCollected;
} Objective;

static BounceEffect sBounceEffects[MAX_BOUNCE_EFFECTS];
static float sTimeSinceBounce;

static Objective sObjectives[OBJECTIVE_GROUP_SIZE];
static float sObjectiveDelayTime;
static int sCollectedObjectives;
static int sHighScoreObjectives;

static Vector2 sBallVelocity;
static Vector2 sBallPosition;

static Vector2 sPlayerPosition;
static Vector2 sPlayerVelocity;
static Vector2 sPlayerSize;

static GameState sCurrentGameState;
static ObjectiveState sCurrentObjectiveState;

INCBIN(BallHitSound, "sfx_ball_hit.wav");
INCBIN(ObjectiveCollectSound, "sfx_objective_collect.wav");
INCBIN(RestartSound, "sfx_scratch.wav");

static Sound sBallHitSound;
static Sound sRestartSound;
static Sound sObjectiveCollectSound;

static void HandleBounce(Vector2 position);
static void ChangeGameStateTo(GameState newState);
static void ChangeObjectiveStateTo(ObjectiveState newState);

static Sound LoadSoundFromMemory(
    const char *fileType, const unsigned char *data, int dataSize
) {
    Wave wave = LoadWaveFromMemory(fileType, data, dataSize);
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

int main(void) {
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "PONG");

    InitAudioDevice();
    sBallHitSound = LoadSoundFromMemory(
        ".wav", gBallHitSoundData, (int) gBallHitSoundSize);
    sRestartSound = LoadSoundFromMemory(
        ".wav", gRestartSoundData, (int) gRestartSoundSize);
    sObjectiveCollectSound = LoadSoundFromMemory(
        ".wav", gObjectiveCollectSoundData, (int) gObjectiveCollectSoundSize);

    ChangeGameStateTo(GamePlaying);

    while (!WindowShouldClose()) {
        switch (sCurrentGameState) {
            case GamePlaying: {
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText(TextFormat("%u collected", sCollectedObjectives),
                         190, 200, 20, WHITE);
                DrawText(TextFormat("%u highscore", sHighScoreObjectives),
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

                // render objectives
                for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
                    float size = sObjectives[i].size;

                    // construct triangle in local space
                    Vector2 position = sObjectives[i].position;
                    Vector2 vertexA = {.x = 0, .y = 0.43f};
                    Vector2 vertexB = {.x = -0.5f, .y = -0.43f};
                    Vector2 vertexC = {.x = 0.5f, .y = -0.43f};

                    // apply transformations to triangle
                    Matrix matrix = MatrixIdentity();
                    float rotation = (float) GetTime() * OBJECTIVE_ROTATE_SPEED;

                    matrix = MatrixMultiply(matrix, MatrixScale(size, size, size));
                    matrix = MatrixMultiply(matrix, MatrixRotateZ(rotation));
                    matrix = MatrixMultiply(matrix, MatrixTranslate(position.x, position.y, 0));
                    vertexA = Vector2Transform(vertexA, matrix);
                    vertexB = Vector2Transform(vertexB, matrix);
                    vertexC = Vector2Transform(vertexC, matrix);

                    // render triangle
                    DrawTriangle(vertexC, vertexB, vertexA, YELLOW);
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

                // update objective size
                for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
                    float targetSize = sObjectives[i].isCollected ? 0.0f : OBJECTIVE_SIZE;
                    sObjectives[i].size = Lerp(sObjectives[i].size, targetSize, OBJECTIVE_ANIM_TIME * deltaTime);
                }

                // objective logic
                switch (sCurrentObjectiveState) {
                    case ObjectivesActive: {
                        // check collision
                        for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
                            if (!sObjectives[i].isCollected && CheckCollisionCircleRec(sObjectives[i].position, OBJECTIVE_SIZE, playerRect)) {
                                PlaySound(sObjectiveCollectSound);
                                sObjectives[i].isCollected = true;
                                sCollectedObjectives++;
                                if (sCollectedObjectives > sHighScoreObjectives) {
                                    sHighScoreObjectives = sCollectedObjectives;
                                }

                                // check to see if we collected everything
                                int remainingObjectives = OBJECTIVE_GROUP_SIZE;

                                for (int j = 0; j < OBJECTIVE_GROUP_SIZE; ++j) {
                                    if (sObjectives[j].isCollected) {
                                        remainingObjectives--;
                                    }
                                }

                                if (remainingObjectives == 0) {
                                    ChangeObjectiveStateTo(ObjectivesDelayed);
                                }
                            }
                        }
                        break;
                    }
                    case ObjectivesDelayed: {
                        sObjectiveDelayTime -= deltaTime;
                        if (sObjectiveDelayTime <= 0) {
                            ChangeObjectiveStateTo(ObjectivesActive);
                        }
                        break;
                    }
                }

                // end frame

                EndDrawing();
                break;
            }

            case GameOver:
                BeginDrawing();
                ClearBackground(BLACK);

                static bool secretEnabled;
                static const char *secretMessages[] = {
                    "Hiiii maeko!!",
                    "This is an easter egg.",
                    "You found it!",
                    "You're the best!!",
                    "- from daniel",
                };
                static int currentMessage;

                DrawText("GAME OVER", GAME_WIDTH / 2, GAME_HEIGHT / 2, 40, WHITE);
                DrawText("press enter to restart", GAME_WIDTH / 2, (GAME_HEIGHT / 2) + 40, 20, WHITE);

                if (secretEnabled) {
                    DrawText(secretMessages[currentMessage], GAME_WIDTH / 2, (GAME_HEIGHT / 2) + 80, 20, GREEN); 
                }

                if (IsKeyPressed(KEY_M)) {
                    if (!secretEnabled) {
                        secretEnabled = true;
                    }
                    else {
                        currentMessage = (currentMessage + 1) % (int) (sizeof(secretMessages) / sizeof(void*));
                    }
                }

                if (IsKeyPressed(KEY_ENTER)) {
                    PlaySound(sRestartSound);
                    ChangeGameStateTo(GamePlaying);
                }

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

    PlaySound(sBallHitSound);

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

            sCollectedObjectives = 0;

            for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
                sBounceEffects[i].remainingTime = 0;
            }
            for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
                sObjectives[i].size = 0;
            }
            ChangeObjectiveStateTo(ObjectivesDelayed);

            break;
        }
        case GameOver: {
            PlaySound(sRestartSound);
            break;
        }
    }
}

static void ChangeObjectiveStateTo(ObjectiveState newState) {
    sCurrentObjectiveState = newState;

    switch (newState) {
        case ObjectivesActive: {
            for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
                sObjectives[i].isCollected = false;
                sObjectives[i].position.x = (float) GetRandomValue(OBJECTIVE_SIZE, GAME_WIDTH - OBJECTIVE_SIZE);
                sObjectives[i].position.y = (float) GetRandomValue(OBJECTIVE_SIZE, GAME_HEIGHT - OBJECTIVE_SIZE);
                sObjectives[i].size = 0;
            }
            break;
        }
        case ObjectivesDelayed: {
            for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
                sObjectives[i].isCollected = true;
            }
            sObjectiveDelayTime = OBJECTIVE_DELAY_TIME;
            break;
        }
    }
}

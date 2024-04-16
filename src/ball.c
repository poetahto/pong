#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include "ball.h"
#include "game.h"
#include "sound.h"
#include "player.h"
#include "math_util.h"

typedef struct BounceEffect {
    float remainingTime;
    Vector2 position;
} BounceEffect;

typedef enum BallState {
    BALL_STATE_SPAWNING,
    BALL_STATE_ACTIVE,
} BallState;

typedef struct BallInstance {
    union {
        struct {
            Vector2 velocity;
            float timeSinceBounce;
        } activeData;
        struct {
            float elapsedTime;
        } spawningData;
    };
    float size;
    Vector2 position;
    Color color;
    BallState state;
} BallInstance;

static void HandleBounce(BallInstance *ball);

static BallInstance sSpawnedBalls[MAX_BALLS];
static BounceEffect sBounceEffects[MAX_BOUNCE_EFFECTS];
static int sSpawnedBallCount;

void SpawnBall() {
    BallInstance newBallInstance = {
        .spawningData = {
            .elapsedTime = 0,
        },
        .position = {
            .x = RandomFloat() * GAME_WIDTH,
            .y = RandomFloat() * GAME_HEIGHT,
        },
        .size = 0,
        .state = BALL_STATE_SPAWNING,
    };
    sSpawnedBalls[sSpawnedBallCount] = newBallInstance;
    sSpawnedBallCount++;
}

void RenderBalls() {
    for (int i = 0; i < sSpawnedBallCount; ++i) {
        DrawCircleV(sSpawnedBalls[i].position, sSpawnedBalls[i].size, WHITE);
    }
}

void InitBalls() {
    sSpawnedBallCount = 0;

    for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
        sBounceEffects[i].remainingTime = 0;
    }
}

void UpdateBalls(float deltaTime) {
    for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
        // skip if not OBJECTIVE_STATE_ACTIVE
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

    for (int i = 0; i < sSpawnedBallCount; ++i) {
        BallInstance *ball = &sSpawnedBalls[i];

        switch (ball->state) {
            case BALL_STATE_SPAWNING: {
                float t = ball->spawningData.elapsedTime / BALL_SPAWN_TIME;
                ball->size = Lerp(0, BALL_SIZE, t);
                ball->spawningData.elapsedTime += GetFrameTime();

                if (t >= 1) {
                    ball->state = BALL_STATE_ACTIVE;
                    ball->activeData.timeSinceBounce = 0;
                    ball->activeData.velocity = Vector2Scale(RandomPointOnUnitCircle(), BALL_SPEED);
                }

                break;
            }
            case BALL_STATE_ACTIVE: {
                // update ball
                ball->activeData.timeSinceBounce += deltaTime;

                // how close are we to going max-speed?
                float velocityPercent = Clamp(ball->activeData.timeSinceBounce / BALL_ACCELERATION_TIME, 0, 1);
                // update velocity based on acceleration rate
                Vector2 initialVelocity = Vector2Scale(
                    Vector2Normalize(ball->activeData.velocity), BALL_SPEED);
                Vector2 targetVelocity = Vector2Scale(
                    Vector2Normalize(ball->activeData.velocity), BALL_MAX_SPEED);
                ball->activeData.velocity = Vector2Lerp(initialVelocity, targetVelocity, velocityPercent);

                // update position based on velocity for this frame
                ball->position = Vector2Add(
                    Vector2Scale(ball->activeData.velocity, deltaTime), ball->position);

                ball->size = Lerp(BALL_SIZE, BALL_MIN_SIZE, velocityPercent);

                // bounce off-screen top/bottom
                if (ball->position.x > GAME_WIDTH || ball->position.x < 0) {
                    ball->activeData.velocity.x = -ball->activeData.velocity.x;
                    ball->position.x = Clamp(ball->position.x, 0, GAME_WIDTH);
                    HandleBounce(ball);
                }

                // bounce off-screen left/right
                if (ball->position.y > GAME_HEIGHT || ball->position.y < 0) {
                    ball->activeData.velocity.y = -ball->activeData.velocity.y;
                    ball->position.y = Clamp(ball->position.y, 0, GAME_HEIGHT);
                    HandleBounce(ball);
                }

                if (CheckCollisionCircleRec(ball->position, ball->size, GetPlayerRect())) {
                    ChangeGameStateTo(GAME_STATE_OVER);
                }
                break;
            }
        }
    }
}

static void HandleBounce(BallInstance *ball) {
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
    sBounceEffects[effectId].position = ball->position;

    // reset ball speed + acceleration
    ball->activeData.timeSinceBounce = 0;
    ball->activeData.velocity = Vector2Scale(Vector2Normalize(ball->activeData.velocity), BALL_SPEED);
}
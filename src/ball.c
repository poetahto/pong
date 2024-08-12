#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include "ball.h"
#include "game.h"
#include "sound.h"
#include "player.h"
#include "math_util.h"
#include "particles.h"

typedef struct BounceEffect {
    float remainingTime;
    Vector2 position;
    Color color;
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
        } active;
        struct {
            float elapsedTime;
        } spawning;
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
        .spawning = {
            .elapsedTime = 0,
        },
        .position = {
            .x = RandomFloat() * GAME_WIDTH,
            .y = RandomFloat() * GAME_HEIGHT,
        },
        .size = 0,
        .color = RandomColor(),
        .state = BALL_STATE_SPAWNING,
    };
    sSpawnedBalls[sSpawnedBallCount] = newBallInstance;
    sSpawnedBallCount++;
}

void RenderBalls() {
    for (int i = 0; i < sSpawnedBallCount; ++i) {
        BallInstance *ball = &sSpawnedBalls[i];

        switch (ball->state) {
            case BALL_STATE_SPAWNING: {
                float spawnPercent = ball->spawning.elapsedTime / BALL_SPAWN_TIME;
                DrawRing(ball->position, ball->size * SmoothStop3(1 - spawnPercent), ball->size, 0, 360, 30, ball->color);
                break;
            }
            case BALL_STATE_ACTIVE: {
                DrawCircleV(ball->position, ball->size, ball->color);
                break;
            }
        }
    }

    for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
        BounceEffect* bounceEffect = &sBounceEffects[i];

        // find the percent complete we are with the effect
        float t = sBounceEffects[i].remainingTime / BOUNCE_EFFECT_DURATION;

        // the size multiplier should start at 1 and end at BOUNCE_EFFECT_MAX_SIZE
        float bounceSizeMultiplier = 1 + ((BOUNCE_EFFECT_MAX_SIZE - 1) * (1 - t));

        // the color should fade out as the effect completes
        Color color = sBounceEffects[i].color;
        color.a = (unsigned char)((float) color.a * t);

        // render the bounce effect
        float outerRadius = BALL_SIZE * bounceSizeMultiplier;
        float innerRadius = fmaxf(outerRadius - BOUNCE_EFFECT_WIDTH, 0);
        DrawRing(bounceEffect->position, innerRadius, outerRadius, 0, 360, 30, color);
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
    }

    for (int i = 0; i < sSpawnedBallCount; ++i) {
        BallInstance *ball = &sSpawnedBalls[i];

        switch (ball->state) {
            case BALL_STATE_SPAWNING: {
                float t = ball->spawning.elapsedTime / BALL_SPAWN_TIME;
                ball->size = Lerp(0, BALL_SIZE, t);
                ball->spawning.elapsedTime += GetFrameTime();

                if (t >= 1) {
                    ball->state = BALL_STATE_ACTIVE;
                    ball->active.timeSinceBounce = 0;
                    ball->active.velocity = Vector2Scale(RandomPointOnUnitCircle(), BALL_SPEED);
                }

                break;
            }
            case BALL_STATE_ACTIVE: {
                // update ball
                ball->active.timeSinceBounce += deltaTime;

                // how close are we to going max-speed?
                float velocityPercent = Clamp(ball->active.timeSinceBounce / BALL_ACCELERATION_TIME, 0, 1);
                // update velocity based on acceleration rate
                Vector2 initialVelocity = Vector2Scale(
                    Vector2Normalize(ball->active.velocity), BALL_SPEED);
                Vector2 targetVelocity = Vector2Scale(
                    Vector2Normalize(ball->active.velocity), BALL_MAX_SPEED);
                ball->active.velocity = Vector2Lerp(initialVelocity, targetVelocity, velocityPercent);

                // update position based on velocity for this frame
                ball->position = Vector2Add(
                    Vector2Scale(ball->active.velocity, deltaTime), ball->position);

                ball->size = Lerp(BALL_SIZE, BALL_MIN_SIZE, velocityPercent);

                // bounce off-screen top/bottom
                if (ball->position.x > GAME_WIDTH || ball->position.x < 0) {
                    ball->active.velocity.x = -ball->active.velocity.x;
                    ball->position.x = Clamp(ball->position.x, 0, GAME_WIDTH);
                    HandleBounce(ball);
                }

                // bounce off-screen left/right
                if (ball->position.y > GAME_HEIGHT || ball->position.y < 0) {
                    ball->active.velocity.y = -ball->active.velocity.y;
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
    BounceEffect *bounceEffect = NULL;

    for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
        bounceEffect = &sBounceEffects[i];
        if (bounceEffect->remainingTime <= 0) {
            break;
        }
    }

    if (bounceEffect == NULL) {
        printf("ran out of bounce effects!\n");
        return;
    }

    PlaySound(gBallHitSound);

    // initialize new bounce effect
    bounceEffect->remainingTime = BOUNCE_EFFECT_DURATION;
    bounceEffect->position = ball->position;
    bounceEffect->color = ball->color;

    // reset ball speed + acceleration
    ball->active.timeSinceBounce = 0;
    ball->active.velocity = Vector2Scale(Vector2Normalize(ball->active.velocity), BALL_SPEED);
}
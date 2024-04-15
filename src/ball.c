#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include "ball.h"
#include "game.h"
#include "sound.h"
#include "player.h"

static void HandleBounce(Vector2 position);

typedef struct BounceEffect {
    float remainingTime;
    Vector2 position;
} BounceEffect;

static BounceEffect sBounceEffects[MAX_BOUNCE_EFFECTS];
static float sTimeSinceBounce;

static Vector2 sBallVelocity;
static Vector2 sBallPosition;
static float sBallSize;

void RenderBalls() {
    DrawCircleV(sBallPosition, sBallSize, WHITE);
}

void InitBalls() {
    sBallVelocity.x = BALL_SPEED;
    sBallVelocity.y = BALL_SPEED;

    sBallPosition.x = GAME_WIDTH / 2.0f;
    sBallPosition.y = GAME_HEIGHT / 2.0f;

    for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
        sBounceEffects[i].remainingTime = 0;
    }
}

void UpdateBalls(float deltaTime) {
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

    sBallSize = Lerp(BALL_SIZE, BALL_MIN_SIZE, velocityPercent);

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

    if (CheckCollisionCircleRec(sBallPosition, sBallSize, GetPlayerRect())) {
        ChangeGameStateTo(GameOver);
    }
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
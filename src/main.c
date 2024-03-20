// Random small game project w/ raylib
// - Daniel Walls, 3/19/2024
// todo: ideas
// - particles on bounce hit
// - player death w/ getting hit
// - sfx
// - timer for staying alive
// - multiple balls of diff color and diff colored effects sizes

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#define GAME_WIDTH 800
#define GAME_HEIGHT 600

#define BALL_SIZE 25 // in pixels
#define BALL_MIN_SIZE 6 // in pixels
#define BALL_SPEED 10 // in pixels per second
#define BALL_MAX_SPEED 2000 // in pixels per second
#define BALL_ACCELERATION 1000 // in pixels per second per second
#define ACCELERATION_TIME 0.5f // in seconds

#define MAX_BOUNCE_EFFECTS 16
#define BOUNCE_EFFECT_DURATION 1.5f // in seconds
#define BOUNCE_EFFECT_MAX_SIZE 5 // multiple of original size
#define BOUNCE_EFFECT_WIDTH 2 // in pixels

#define PLAYER_WIDTH 25 // in pixels
#define PLAYER_HEIGHT 28 // in pixels
#define PLAYER_SPEED 500 // in pixels per second
#define PLAYER_ACCELERATION 4000 // in pixels per second per second
#define PLAYER_DECCELERATION 15 // in weird lerp units LOL
#define PLAYER_SQUISH_AMOUNT 0.6f // in percent size

typedef struct BounceEffect {
    float remainingTime;
    Vector2 position;
} BounceEffect;

static BounceEffect sBounceEffects[MAX_BOUNCE_EFFECTS];
static float sTimeSinceBounce;
static Vector2 sBallVelocity;

static void HandleBounce(Vector2 position);

int main(void) {
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "PONG");

    sBallVelocity.x = BALL_SPEED;    
    sBallVelocity.y = BALL_SPEED;    

    Vector2 ballPosition = {
        .x = GAME_WIDTH / 2,
        .y = GAME_HEIGHT / 2,
    };

    Vector2 playerPosition = {
        .x = GAME_WIDTH / 2,
        .y = GAME_HEIGHT / 2,
    };

    Vector2 playerVelocity = Vector2Zero();

    Vector2 playerSize = {
        .x = PLAYER_WIDTH,
        .y = PLAYER_HEIGHT,
    };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Hello, world.", 190, 200, 20, WHITE);
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
            playerVelocity = Vector2MoveTowards(
                playerVelocity, targetVelocity, PLAYER_ACCELERATION * deltaTime
            );
        }
        else { // is deccelerating
            playerVelocity = Vector2Lerp(
                playerVelocity, Vector2Zero(), PLAYER_DECCELERATION * deltaTime
            );
        }

        playerPosition = Vector2Add(
            playerPosition, Vector2Scale(playerVelocity, deltaTime)
        );

        playerPosition.x = Clamp(playerPosition.x, 0, GAME_WIDTH - PLAYER_WIDTH);
        playerPosition.y = Clamp(playerPosition.y, 0, GAME_HEIGHT - PLAYER_WIDTH);

        // animate size based on speed
        playerSize.y = Lerp(
            PLAYER_WIDTH, 
            PLAYER_WIDTH * PLAYER_SQUISH_AMOUNT, 
            fabs(playerVelocity.x) / PLAYER_SPEED
        );
        playerSize.x = Lerp(
            PLAYER_HEIGHT, 
            PLAYER_HEIGHT * PLAYER_SQUISH_AMOUNT, 
            fabs(playerVelocity.y) / PLAYER_SPEED
        );

        // update bounce effects

        for (int i = 0; i < MAX_BOUNCE_EFFECTS; ++i) {
            // skip if not active
            if (sBounceEffects[i].remainingTime <= 0) {
                continue;
            }
            sBounceEffects[i].remainingTime -= deltaTime;

            // find the percent complete we are with the effect
            float t = sBounceEffects[i].remainingTime / BOUNCE_EFFECT_DURATION;

            // the size multiplier should start at 1 and end at BOUNCE_EFFECT_MAX_SIZE
            float bounceSizeMultiplier = 1 + ((BOUNCE_EFFECT_MAX_SIZE - 1) * (1 - t));

            // the color should fade out as the effect completes
            Color color = WHITE;
            color.a = (unsigned char) (color.a * t);

            // render the bounce effect
            DrawCircleV( // outer circle
                sBounceEffects[i].position, 
                BALL_SIZE * bounceSizeMultiplier, 
                color
            );
            DrawCircleV( // inner circle (we want to be at most slightly smaller)
                sBounceEffects[i].position, 
                max(((float) BALL_SIZE * bounceSizeMultiplier) - BOUNCE_EFFECT_WIDTH, 0), 
                BLACK
            );
        }
        
        // update ball
    
        sTimeSinceBounce += deltaTime;

        // how close are we to going max-speed?
        float velocityPercent = Clamp(sTimeSinceBounce / ACCELERATION_TIME, 0, 1);

        // update velocity based on acceleration rate
        Vector2 initialVelocity = Vector2Scale(
            Vector2Normalize(sBallVelocity), BALL_SPEED);
        Vector2 targetVelocity = Vector2Scale(
            Vector2Normalize(sBallVelocity), BALL_MAX_SPEED);
        sBallVelocity = Vector2Lerp(initialVelocity, targetVelocity, velocityPercent);

        // update position based on velocity for this frame
        ballPosition = Vector2Add(Vector2Scale(sBallVelocity, deltaTime), ballPosition);

        float curBallSize = Lerp(BALL_SIZE, BALL_MIN_SIZE, velocityPercent);

        // bounce off screen top/bottom
        if (ballPosition.x > GAME_WIDTH || ballPosition.x < 0) {
            sBallVelocity.x = -sBallVelocity.x;
            ballPosition.x = Clamp(ballPosition.x, 0, GAME_WIDTH);
            HandleBounce(ballPosition);
        }

        // bounce off screen left/right
        if (ballPosition.y > GAME_HEIGHT || ballPosition.y < 0) {
            sBallVelocity.y = -sBallVelocity.y;
            ballPosition.y = Clamp(ballPosition.y, 0, GAME_HEIGHT);
            HandleBounce(ballPosition);
        }
        
        // render ball
        DrawCircleV(ballPosition, curBallSize, WHITE);

        // render player
        DrawRectangleV(playerPosition, playerSize, WHITE);

        // end frame

        EndDrawing();
    }

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

    // initialize new bounce effect
    sBounceEffects[effectId].remainingTime = BOUNCE_EFFECT_DURATION;
    sBounceEffects[effectId].position = position;

    // reset ball speed + acceleration
    sTimeSinceBounce = 0;
    sBallVelocity = Vector2Scale(Vector2Normalize(sBallVelocity), BALL_SPEED);
}


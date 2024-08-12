#include <raylib.h>
#include <raymath.h>
#include "player.h"
#include "game.h"

Vector2 gPlayerPosition;

static Vector2 sPlayerVelocity;
static Vector2 sPlayerSize;

static Vector2 GetPlayerTopLeftCorner() {
    Vector2 topLeft = gPlayerPosition;
    topLeft.x -= 0.5f * sPlayerSize.x;
    topLeft.y -= 0.5f * sPlayerSize.y;
    return topLeft;
}

void InitPlayer() {
    gPlayerPosition.x = GAME_WIDTH / 2.0f;
    gPlayerPosition.y = (GAME_HEIGHT / 2.0f) + 100;
}

void RenderPlayer() {
    DrawRectangleV(GetPlayerTopLeftCorner(), sPlayerSize, WHITE);
}

Rectangle GetPlayerRect() {
    Vector2 topLeft = GetPlayerTopLeftCorner();
    Rectangle playerRect = {
        .x = topLeft.x,
        .y = topLeft.y,
        .width = sPlayerSize.x,
        .height = sPlayerSize.y,
    };
    return playerRect;
}

void UpdatePlayer(float deltaTime) {
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
        sPlayerVelocity = Vector2MoveTowards(sPlayerVelocity, targetVelocity, PLAYER_ACCELERATION * deltaTime);
    }
    else { // is decelerating
        sPlayerVelocity = Vector2Lerp(sPlayerVelocity, Vector2Zero(), PLAYER_DECELERATION * deltaTime);
    }

    gPlayerPosition = Vector2Add(gPlayerPosition, Vector2Scale(sPlayerVelocity, deltaTime));

    gPlayerPosition.x = Clamp(
        gPlayerPosition.x, 
        PLAYER_WIDTH * 0.5f, 
        GAME_WIDTH - PLAYER_WIDTH * 0.5f
    );
    gPlayerPosition.y = Clamp(
        gPlayerPosition.y, 
        PLAYER_HEIGHT * 0.5f, 
        GAME_HEIGHT - PLAYER_HEIGHT * 0.5f
    );

    // animate size based on speed
    sPlayerSize.y = Lerp(
        PLAYER_WIDTH,
        PLAYER_WIDTH * PLAYER_SQUISH_AMOUNT,
        fabsf(sPlayerVelocity.x) / PLAYER_SPEED
    );
    sPlayerSize.x = Lerp(
        PLAYER_HEIGHT,
        PLAYER_HEIGHT * PLAYER_SQUISH_AMOUNT,
        fabsf(sPlayerVelocity.y) / PLAYER_SPEED
    );
}

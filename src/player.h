#ifndef PONG_PLAYER_H
#define PONG_PLAYER_H

#define PLAYER_WIDTH 25           // in pixels
#define PLAYER_HEIGHT 28          // in pixels
#define PLAYER_SPEED 500          // in pixels per second
#define PLAYER_ACCELERATION 4000  // in pixels per second per second
#define PLAYER_DECELERATION 15   // in weird lerp units LOL
#define PLAYER_SQUISH_AMOUNT 0.4f // in percent size

void UpdatePlayer();

#endif // PONG_PLAYER_H
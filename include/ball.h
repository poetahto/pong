#ifndef PONG_BALL_H
#define PONG_BALL_H

#define MAX_BALLS 10
#define BALL_SIZE 35                // in pixels
#define BALL_MIN_SIZE 15            // in pixels
#define BALL_SPEED 10               // in pixels per second
// #define BALL_MAX_SPEED 2000         // in pixels per second
#define BALL_MAX_SPEED 250         // in pixels per second
#define BALL_ACCELERATION_TIME 0.5f // in seconds
#define BALL_SPAWN_TIME 1           // in seconds

#define MAX_BOUNCE_EFFECTS 16
#define BOUNCE_EFFECT_DURATION 1.5f // in seconds
#define BOUNCE_EFFECT_MAX_SIZE 5    // multiple of original size
#define BOUNCE_EFFECT_WIDTH 2       // in pixels

void InitBalls();
void UpdateBalls(float deltaTime);
void RenderBalls();
void SpawnBall();

#endif // PONG_BALL_H
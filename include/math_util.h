#ifndef PONG_MATH_H
#define PONG_MATH_H

#include <raylib.h>
#include <raymath.h>

float RandomFloat();
Vector2 RandomPointOnUnitCircle();
Color RandomColor();

Vector2 GetRectPosition(Rectangle rect);

float SmoothStop2(float t);
float SmoothStop3(float t);
float SmoothStop4(float t);
float SmoothStart2(float t);
float SmoothStart3(float t);
float SmoothStart4(float t);

#endif // PONG_MATH_H
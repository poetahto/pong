#include <stdlib.h>
#include <raylib.h>
#include "math_util.h"

float RandomFloat() {
    return (float) rand() / (float) RAND_MAX;
}

Vector2 RandomPointOnUnitCircle() {
    Vector2 result = {
        .x = (RandomFloat() * 2) - 1,
        .y = (RandomFloat() * 2) - 1,
    };
    result = Vector2Normalize(result);
    return result;
}

Color RandomColor() {
    Color result = {
        .r = RandomFloat() * 255.0f,
        .g = RandomFloat() * 255.0f,
        .b = RandomFloat() * 255.0f,
        .a = 255.0f,
    };
    return result;
}

Vector2 GetRectPosition(Rectangle rect) {
    Vector2 result = {
        .x = rect.x,
        .y = rect.y,
    };
    return result;
}

float SmoothStop2(float t) { return 1 - (1 - t) * (1 - t);}
float SmoothStop3(float t) { return 1 - (1 - t) * (1 - t) * (1 - t);}
float SmoothStop4(float t) { return 1 - (1 - t) * (1 - t) * (1 - t) * (1 - t);}
float SmoothStart2(float t) { return t * t;}
float SmoothStart3(float t) { return t * t * t;}
float SmoothStart4(float t) { return t * t * t * t;}
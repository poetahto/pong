#include <stdlib.h>
#include <raylib.h>
#include "math_util.h"

float RandomFloat() {
    return (float) rand() / (float) RAND_MAX;
}

Vector2 RandomPointOnUnitCircle() {
    Vector2 result = {
        .x = RandomFloat(),
        .y = RandomFloat(),
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
#include <stdlib.h>
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
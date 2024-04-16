#include <raylib.h>
#include <raymath.h>
#include <incbin.h>
#include "objective.h"
#include "sound.h"
#include "game.h"
#include "player.h"
#include "particles.h"

#define OBJECTIVE_GROUP_SIZE 3
#define OBJECTIVE_SIZE 40        // in pixels
#define OBJECTIVE_ANIM_TIME 15   // in weight lerp units
#define OBJECTIVE_DELAY_TIME 1   // in seconds
#define OBJECTIVE_ROTATE_SPEED 2 // in degrees per second

typedef struct Objective {
    Vector2 position;
    float size;
    bool isCollected;
} Objective;

int gCollectedObjectives;
int gHighScoreObjectives;

static Objective sObjectives[OBJECTIVE_GROUP_SIZE];
static float sObjectiveDelayTime;
static ObjectiveState sCurrentObjectiveState;

void InitObjectives() {
    gCollectedObjectives = 0;
    for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
        sObjectives[i].size = 0;
    }
    ChangeObjectiveStateTo(OBJECTIVE_STATE_DELAYED);
}

void ChangeObjectiveStateTo(ObjectiveState state) {
    sCurrentObjectiveState = state;

    switch (state) {
        case OBJECTIVE_STATE_ACTIVE: {
            for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
                sObjectives[i].isCollected = false;
                sObjectives[i].position.x = (float) GetRandomValue(OBJECTIVE_SIZE, GAME_WIDTH - OBJECTIVE_SIZE);
                sObjectives[i].position.y = (float) GetRandomValue(OBJECTIVE_SIZE, GAME_HEIGHT - OBJECTIVE_SIZE);
                sObjectives[i].size = 0;
            }
            break;
        }
        case OBJECTIVE_STATE_DELAYED: {
            for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
                sObjectives[i].isCollected = true;
            }
            sObjectiveDelayTime = OBJECTIVE_DELAY_TIME;
            break;
        }
    }
}

void UpdateObjectives(float deltaTime) {
    // update objective size
    for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
        float targetSize = sObjectives[i].isCollected ? 0.0f : OBJECTIVE_SIZE;
        sObjectives[i].size = Lerp(sObjectives[i].size, targetSize, OBJECTIVE_ANIM_TIME * deltaTime);
    }

    // state logic
    switch (sCurrentObjectiveState) {
        case OBJECTIVE_STATE_ACTIVE: {
            // check collision
            for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
                if (!sObjectives[i].isCollected && CheckCollisionCircleRec(sObjectives[i].position, OBJECTIVE_SIZE, GetPlayerRect())) {
                    PlaySound(gObjectiveCollectSound);
                    PlayParticleBurst(sObjectives[i].position, YELLOW, 5);
                    sObjectives[i].isCollected = true;
                    gCollectedObjectives++;

                    // check to see if we collected everything
                    int remainingObjectives = OBJECTIVE_GROUP_SIZE;

                    for (int j = 0; j < OBJECTIVE_GROUP_SIZE; ++j) {
                        if (sObjectives[j].isCollected) {
                            remainingObjectives--;
                        }
                    }

                    if (remainingObjectives == 0) {
                        ChangeObjectiveStateTo(OBJECTIVE_STATE_DELAYED);
                    }
                }
            }
            break;
        }
        case OBJECTIVE_STATE_DELAYED: {
            sObjectiveDelayTime -= deltaTime;
            if (sObjectiveDelayTime <= 0) {
                ChangeObjectiveStateTo(OBJECTIVE_STATE_ACTIVE);
            }
            break;
        }
    }
}

void RenderObjectives() {
    bool isSettingHighscore = gCollectedObjectives > gHighScoreObjectives;
    DrawText(TextFormat("%u collected", gCollectedObjectives), 190, 200, 20, isSettingHighscore ? GREEN : RED);
    DrawText(TextFormat("%u highscore", gHighScoreObjectives), 190, 180, 20, WHITE);

    for (int i = 0; i < OBJECTIVE_GROUP_SIZE; ++i) {
        float size = sObjectives[i].size;

        // construct triangle in local space
        Vector2 position = sObjectives[i].position;
        Vector2 vertexA = {.x = 0, .y = 0.43f};
        Vector2 vertexB = {.x = -0.5f, .y = -0.43f};
        Vector2 vertexC = {.x = 0.5f, .y = -0.43f};

        // apply transformations to triangle
        Matrix matrix = MatrixIdentity();
        float rotation = (float) GetTime() * OBJECTIVE_ROTATE_SPEED;

        matrix = MatrixMultiply(matrix, MatrixScale(size, size, size));
        matrix = MatrixMultiply(matrix, MatrixRotateZ(rotation));
        matrix = MatrixMultiply(matrix, MatrixTranslate(position.x, position.y, 0));
        vertexA = Vector2Transform(vertexA, matrix);
        vertexB = Vector2Transform(vertexB, matrix);
        vertexC = Vector2Transform(vertexC, matrix);

        // render triangle
        DrawTriangle(vertexC, vertexB, vertexA, YELLOW);
    }
}

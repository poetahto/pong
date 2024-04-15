#ifndef PONG_OBJECTIVE_H
#define PONG_OBJECTIVE_H

extern int gCollectedObjectives;
extern int gHighScoreObjectives;

typedef enum ObjectiveState {
    ObjectivesInitialized,
    ObjectivesActive,
    ObjectivesDelayed
} ObjectiveState;

void ChangeObjectiveStateTo(ObjectiveState state);
void UpdateObjectives(float deltaTime, Rectangle playerRect);
void RenderObjectives();

#endif // PONG_OBJECTIVE_H

#ifndef PONG_OBJECTIVE_H
#define PONG_OBJECTIVE_H

extern int gCollectedObjectives;
extern int gHighScoreObjectives;

typedef enum ObjectiveState {
    ObjectivesActive,
    ObjectivesDelayed
} ObjectiveState;

void InitObjectives();
void ChangeObjectiveStateTo(ObjectiveState state);
void UpdateObjectives(float deltaTime);
void RenderObjectives();

#endif // PONG_OBJECTIVE_H

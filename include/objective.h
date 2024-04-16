#ifndef PONG_OBJECTIVE_H
#define PONG_OBJECTIVE_H

extern int gCollectedObjectives;
extern int gHighScoreObjectives;

typedef enum ObjectiveState {
    OBJECTIVE_STATE_ACTIVE,
    OBJECTIVE_STATE_DELAYED
} ObjectiveState;

void InitObjectives();
void ChangeObjectiveStateTo(ObjectiveState state);
void UpdateObjectives(float deltaTime);
void RenderObjectives();

#endif // PONG_OBJECTIVE_H

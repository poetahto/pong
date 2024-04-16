#ifndef PONG_GAME_H
#define PONG_GAME_H

#define GAME_WIDTH 800
#define GAME_HEIGHT 600

typedef enum GameState {
    GAME_STATE_PLAYING,
    GAME_STATE_OVER
} GameState;

void RunGame();
void ChangeGameStateTo(GameState newState);

#endif // PONG_GAME_H
// Random small game project w/ raylib
// - Daniel Walls, 3/19/2024

// todo: ideas
// - music
// - background particles
// - particles on bounce hit (free list impl?)
// - multiple balls of diff color and diff colored effects sizes
// - ball leaves a trail that you must dodge
// - start to split things into different files...
// - data-oriented approach rather than hard code defines?

// - turn this into a touhou style boss fight?
//  - text box w/ dialogue about the beauty of flow, only when ur near death are you truly alive, wasting time
//  - health bar
//  - stages + attack patterns

#include <raylib.h>
#include "game.h"
#include "sound.h"

int main(void) {
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "PONG");
    InitAudioDevice();
    LoadSounds();

    ChangeGameStateTo(GamePlaying);

    while (!WindowShouldClose()) {
        RunGame();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

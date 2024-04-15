#include <raylib.h>
#include <incbin.h>
#include "sound.h"

Sound gBallHitSound;
INCBIN(BallHitSound, "sfx_ball_hit.wav");

Sound gRestartSound;
INCBIN(ObjectiveCollectSound, "sfx_objective_collect.wav");

Sound gObjectiveCollectSound;
INCBIN(RestartSound, "sfx_scratch.wav");

#define LOAD_AUDIO(NAME) LoadSoundFromMemory(".wav", g ## NAME ## Data, (int) g ## NAME ## Size)

static Sound LoadSoundFromMemory(
    const char *fileType, const unsigned char *data, int dataSize
) {
    Wave wave = LoadWaveFromMemory(fileType, data, dataSize);
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

void LoadSounds() {
    gBallHitSound = LOAD_AUDIO(BallHitSound);
    gRestartSound = LOAD_AUDIO(RestartSound);
    gObjectiveCollectSound = LOAD_AUDIO(ObjectiveCollectSound);
}
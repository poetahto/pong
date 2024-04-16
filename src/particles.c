#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include "particles.h"
#include "math_util.h"

#define MAX_PARTICLES 100
#define MAX_PARTICLE_BURSTS 10 
#define BURST_DURATION 1          // in seconds
#define PARTICLE_SIZE 5           // in pixels
#define PARTICLE_SPEED 100        // in pixels per second

typedef struct ParticleInstance {
    Vector2 position;
    Vector2 velocity;
} ParticleInstance;

typedef struct ParticleBurstInstance {
    ParticleInstance particles[MAX_PARTICLES];
    int particleCount;
    Color color;
    float elapsedTime;
} ParticleBurstInstance;

static ParticleBurstInstance sParticleBursts[MAX_PARTICLE_BURSTS];

void PlayParticleBurst(Vector2 position, Color color, int amount) {
    for (int i = 0; i < MAX_PARTICLE_BURSTS; ++i) {
        ParticleBurstInstance *burst = &sParticleBursts[i];
        if (burst->elapsedTime == -1) {
            burst->elapsedTime = 0;
            burst->color = color;
            burst->particleCount = amount;
            for (int j = 0; j < amount; ++j) {
                burst->particles[j].position = position;
                burst->particles[j].velocity = Vector2Scale(RandomPointOnUnitCircle(), PARTICLE_SPEED);
            } 
            return;
        }
    }
    printf("RAN OUT OF PARTICLES\n");
}

void UpdateParticles(float deltaTime) {
    for (int i = 0; i < MAX_PARTICLE_BURSTS; ++i) {
        ParticleBurstInstance *burst = &sParticleBursts[i];

        // invalid burst
        if (burst->elapsedTime == -1) {
            continue;
        }

        burst->elapsedTime += deltaTime;
        float percentComplete = burst->elapsedTime / BURST_DURATION;
        burst->color.a = Lerp(255.0f, 0.0f, percentComplete);

        for (int j = 0; j < burst->particleCount; ++j) {
            ParticleInstance *particle = &burst->particles[j];
            Vector2 frameVelocity = Vector2Scale(particle->velocity, deltaTime);
            particle->position = Vector2Add(particle->position, frameVelocity);
        }

        if (percentComplete >= 1) {
            burst->elapsedTime = -1;
        }
    }
}

void RenderParticles() {
    Vector2 particleSize = {
        .x = PARTICLE_SIZE,
        .y = PARTICLE_SIZE,
    };
    for (int i = 0; i < MAX_PARTICLE_BURSTS; ++i) {
        ParticleBurstInstance *burst = &sParticleBursts[i];

        // invalid burst
        if (burst->elapsedTime == -1) {
            continue;
        }

        for (int j = 0; j < burst->particleCount; ++j) {
            ParticleInstance *particle = &burst->particles[j];
            DrawRectangleV(particle->position, particleSize, burst->color);
        }
    }
}
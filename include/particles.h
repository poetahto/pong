#ifndef PONG_PARTICLES_H
#define PONG_PARTICLES_H

#include <raylib.h>

void PlayParticleBurst(Vector2 position, Color color, int amount);
void UpdateParticles(float deltaTime);
void RenderParticles();

#endif // PONG_PARTICLES_H
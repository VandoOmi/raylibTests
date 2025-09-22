#ifndef DRAW_H
#define DRAW_H

#include "particle.h"

void DrawParticles(ObjectList* objList, const Camera3D* camera);
void InitParticleRender(void);
void ShutdownParticleRender(void);

#endif
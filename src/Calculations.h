#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include "particle.h"

// Gravity & Movement
void ComputeGravitationWithShader(ObjectList* objList, float deltaTime);

// Collision
void CalculateCollision(ObjectList* list, int particleRadius);

// Runtime toggles
void SetUseGPU(int enabled);
int  IsUseGPU(void);
void SetCullingEnabled(int enabled);
int  IsCullingEnabled(void);

#endif
#ifndef PARTICLE_H
#define PARTICLE_H

#include <raylib.h> 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "settings.h"


enum element{
    hydrogen = 37659,
    helium = 74564,
    oxygen = 598608,
    carbon = 949646300,
    neon = 376968,
    iron = 3298418600
};

typedef struct GravitationalObject {
    const char* name;
    enum element element;
    Vector3 position;
    Vector3 force;
    Vector3 velocity;
} GravitationalObject;

typedef struct ObjectList {
    GravitationalObject** gObjs;
    int size;
} ObjectList;

ObjectList* createObjectList();
void addObjectList(GravitationalObject* obj, ObjectList* objList);
void freeObjectList(ObjectList* objList);

void randomObjectsFor(int count, ObjectList* objList, Vector3 room);
GravitationalObject* createRandomParticleAt(Vector3* pos);

// Input Prossesing
void handleInput(ObjectList* objList, Camera3D* camera);

//Changing Data
void CalculateGravitation(ObjectList* objList);
void ComputeGravitationWithShader(ObjectList* objList, float deltaTime);
void MoveParticles(ObjectList* objList, float deltaTime);
void CalculateCollision(ObjectList* objList);

//Drawing
void DrawParticles(ObjectList* objList, const Camera3D* camera);
void InitParticleRender(void);
void ShutdownParticleRender(void);

// Runtime toggles
void SetUseGPU(int enabled);
int  IsUseGPU(void);
void SetCullingEnabled(int enabled);
int  IsCullingEnabled(void);

//Util
float rand_range(float min, float max);

#endif
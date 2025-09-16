#ifndef PARTICLE_H
#define PARTICLE_H

#include <raylib.h> 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG_MODE 0

typedef struct GravitationalObject GravitationalObject;
typedef struct ObjectList ObjectList;

ObjectList* createObjectList();
void addObjectList(GravitationalObject* obj, ObjectList* objList);
void freeObjectList(ObjectList* objList);

void randomObjectsFor(int count, ObjectList* objList, Vector3 room);
GravitationalObject* createRandomParticleAt(Vector3* pos);

// Input Prossesing
void handleInput(ObjectList* objList, Camera3D* camera);

//Changing Data
void CalculateGravitation(ObjectList* objList);
void MoveParticles(ObjectList* objList, float deltaTime);
void CalculateCollision(ObjectList* objList);

//Drawing
void DrawParticles(ObjectList* objList);

//Util
float rand_range(float min, float max);

#endif
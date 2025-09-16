#ifndef PARTICLE_H
#define PARTICLE_H

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define DEBUG_MODE 0

typedef struct GravitationalObject GravitationalObject;
typedef struct ObjectList ObjectList;

ObjectList* createObjectList();
void addObjectList(GravitationalObject* obj, ObjectList* objList);
void freeObjectList(ObjectList* objList);

void randomObjectsFor(int count, ObjectList* objList, Vector3 room);

// Input Prossesing
void handleInput(ObjectList* objList, Camera3D* camera);

//Changing Data
void calcGraviation(ObjectList* objList);
void moveObjects(ObjectList* objList, float deltaTime);
void handleCollisions(ObjectList* objList);

//Drawing
void drawParticals(ObjectList* objList);

//Util
float rand_range();



#endif
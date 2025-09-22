#ifndef PARTICLE_H
#define PARTICLE_H


#include "settings.h"
#include "compute.h"
#include "GridSystem.h"
#include "GridSystemGravity_CS.h"

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


//Util
float rand_range(float min, float max);

#endif
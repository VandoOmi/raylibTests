// Forward declarations and type definitions
#include "compute.h"
#include "particle.h"


// Use the compute shader to calculate gravity for all objects
void ComputeGravitationWithShader(ObjectList* oList, float deltaTime) {
    if (oList->size == 0) return;
    // Allocate a temporary GPUObject array
    GPUObject* gpuObjs = malloc(sizeof(GPUObject) * oList->size);
    for (int i = 0; i < oList->size; i++) {
        GravitationalObject* obj = oList->gObjs[i];
        gpuObjs[i].position[0] = obj->position.x;
        gpuObjs[i].position[1] = obj->position.y;
        gpuObjs[i].position[2] = obj->position.z;
        gpuObjs[i].velocity[0] = obj->velocity.x;
        gpuObjs[i].velocity[1] = obj->velocity.y;
        gpuObjs[i].velocity[2] = obj->velocity.z;
        gpuObjs[i].mass = (float)obj->element;
    }
    // Run the compute shader
    computeGravity(gpuObjs, oList->size, deltaTime);
    // Copy results back to GravitationalObject
    for (int i = 0; i < oList->size; i++) {
        GravitationalObject* obj = oList->gObjs[i];
        obj->position.x = gpuObjs[i].position[0];
        obj->position.y = gpuObjs[i].position[1];
        obj->position.z = gpuObjs[i].position[2];
        obj->velocity.x = gpuObjs[i].velocity[0];
        obj->velocity.y = gpuObjs[i].velocity[1];
        obj->velocity.z = gpuObjs[i].velocity[2];
    }
    free(gpuObjs);
}

// Universal gravitational constant
const float G = 6.67430e-11f;
// Particle radius (in km)
const int PARTICLERADIUS = 1;
#define HASH_SIZE 10007

// Used for custom object creation
Vector3 firstPos = { 0, 0, 0 };



// Linked list entry for spatial hash grid
typedef struct CellEntry {
    GravitationalObject* obj;
    struct CellEntry* next;
} CellEntry;

// Spatial hash grid for efficient neighbor search
typedef struct {
    CellEntry* table[HASH_SIZE];
} SpatialHash;

// Hash function for 3D cell coordinates
unsigned int hashCell(int x, int y, int z) {
    unsigned int h = 73856093u * x ^ 19349663u * y ^ 83492791u * z;
    return h % HASH_SIZE;
}

// Insert an object into the spatial hash grid
void insertObject(SpatialHash* grid, GravitationalObject* obj, float cellSize) {
    int cx = (int)floor(obj->position.x / cellSize);
    int cy = (int)floor(obj->position.y / cellSize);
    int cz = (int)floor(obj->position.z / cellSize);
    unsigned int h = hashCell(cx, cy, cz);
    CellEntry* entry = malloc(sizeof(CellEntry));
    entry->obj = obj;
    entry->next = grid->table[h];
    grid->table[h] = entry;
}

// Free all memory used by the spatial hash grid
void freeSpatialHash(SpatialHash* grid) {
    for (int h = 0; h < HASH_SIZE; h++) {
        CellEntry* entry = grid->table[h];
        while (entry) {
            CellEntry* next = entry->next;
            free(entry);
            entry = next;
        }
        grid->table[h] = NULL;
    }
}


// Get color for a given element type
Color getColor(enum element element) {
    switch (element) {
        case hydrogen: return RAYWHITE;
        case helium:   return RED;
        case oxygen:   return BLUE;
        case carbon:   return GRAY;
        case neon:     return PINK;
        case iron:     return LIGHTGRAY;
    }
    return WHITE;
}

// Draw a single particle as a sphere
void drawParticle(GravitationalObject *obj) {
    DrawSphere((Vector3)obj->position, PARTICLERADIUS, getColor(obj->element));
    if (DEBUG_MODE) {
        printf("[DRAW] %s: pos=(%.2f, %.2f, %.2f)\n", obj->name, obj->position.x, obj->position.y, obj->position.z);
    }
}

// Draw all particles in the object list
void DrawParticles(ObjectList* oList) {
    for (int i = 0; i < oList->size; i++) {
        drawParticle(oList->gObjs[i]);
    }
}

// Convert mouse position to a 3D world point at a given distance from the camera
Vector3 GetMouseWorldPoint(const Camera3D *camera, float distance) {
    Ray ray = GetMouseRay(GetMousePosition(), *camera);
    Vector3 point = {
        ray.position.x + ray.direction.x * distance,
        ray.position.y + ray.direction.y * distance,
        ray.position.z + ray.direction.z * distance
    };
    return point;
}

// Calculate gravitational forces between all objects using spatial hashing
void CalculateGravitation(ObjectList* oList) {
    // Reset all forces
    for (int i = 0; i < oList->size; i++) {
        oList->gObjs[i]->force = (Vector3){0.0f, 0.0f, 0.0f};
    }
    float cellSize = 20.f;
    SpatialHash grid = {0};
    // Insert all objects into the spatial grid
    for (int i = 0; i < oList->size; i++) {
        insertObject(&grid, oList->gObjs[i], cellSize);
    }
    // For each cell, compute forces with neighbors
    for (int h = 0; h < HASH_SIZE; h++) {
        CellEntry* entry = grid.table[h];
        while (entry) {
            GravitationalObject* a = entry->obj;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        unsigned int nh = hashCell(
                            (int)floor(a->position.x / cellSize) + dx,
                            (int)floor(a->position.y / cellSize) + dy,
                            (int)floor(a->position.z / cellSize) + dz
                        );
                        CellEntry* neighbor = grid.table[nh];
                        while (neighbor) {
                            GravitationalObject* b = neighbor->obj;
                            if (a == b) { neighbor = neighbor->next; continue; }
                            float dX = b->position.x - a->position.x;
                            float dY = b->position.y - a->position.y;
                            float dZ = b->position.z - a->position.z;
                            float r2 = dX*dX + dY*dY + dZ*dZ;
                            float r = sqrtf(r2);
                            if (r > 1e-10f) {
                                float f = (G * a->element * b->element) / r2;
                                float fx = f * (dX / r);
                                float fy = f * (dY / r);
                                float fz = f * (dZ / r);
                                a->force.x += fx;
                                a->force.y += fy;
                                a->force.z += fz;
                                b->force.x -= fx;
                                b->force.y -= fy;
                                b->force.z -= fz;
                            }
                            neighbor = neighbor->next;
                        }
                    }
                }
            }
            entry = entry->next;
        }
    }
    freeSpatialHash(&grid);
}

// Update position and velocity of a single object using its force
void moveObject(GravitationalObject *obj, float deltaTime) {
    float aX = obj->force.x/obj->element;
    float aY = obj->force.y/obj->element;
    float aZ = obj->force.z/obj->element;
    obj->velocity.x += aX * deltaTime;
    obj->velocity.y += aY * deltaTime;
    obj->velocity.z += aZ * deltaTime;
    obj->position.x += obj->velocity.x * deltaTime;
    obj->position.y += obj->velocity.y * deltaTime;
    obj->position.z += obj->velocity.z * deltaTime;
    if (DEBUG_MODE) {
        printf("[MOVE] %s:\n", obj->name);
        printf("       aX=%.5f, aY=%.5f\n", aX, aY);
        printf("       velX=%.5f, velY=%.5f, velZ=%.5f\n", obj->velocity.x, obj->velocity.y, obj->velocity.z);
        printf("       posX=%.2f, posY=%.2f, posZ=%.2f\n", obj->position.x, obj->position.y, obj->position.z);
    }
}

// Update all particles in the object list
void MoveParticles(ObjectList* oList, float deltaTime) {
    for (int i = 0; i < oList->size; i++) {
        moveObject(oList->gObjs[i], deltaTime);
    }
}

// Create a new, empty object list
ObjectList* createObjectList() {
    ObjectList* list = malloc(sizeof(ObjectList));
    list->gObjs = NULL;
    list->size = 0;
    return list;
}

// Add a new object to the object list
void addObjectList(GravitationalObject* obj, ObjectList* oList) {
    GravitationalObject** temp_array = realloc(oList->gObjs, (oList->size+1) * sizeof(GravitationalObject*));
    if (temp_array == NULL) {
        fprintf(stderr, "[ERROR] Could not allocate memory for new object.\n");
        return;
    }
    oList->gObjs = temp_array;
    oList->gObjs[oList->size] = obj;
    oList->size++;
}

// Free all memory used by the object list and its objects
void freeObjectList(ObjectList* oList) {
    for (int i = 0; i < oList->size; i++) {
        free(oList->gObjs[i]);
    }
    free(oList->gObjs);
    free(oList);
}

// Create a random particle at a given position
GravitationalObject* createRandomParticleAt(Vector3* pos) {
    GravitationalObject* obj = malloc(sizeof(GravitationalObject));
    enum element elements[] = {hydrogen, helium, oxygen, carbon, neon, iron};
    obj->name = "Random";
    obj->element = elements[rand() % 6];
    obj->position = *pos;
    obj->force = (Vector3){0, 0, 0};
    obj->velocity.x = GetRandomValue(-0.1, 0.1);
    obj->velocity.y = GetRandomValue(-0.1, 0.1);
    obj->velocity.z = GetRandomValue(-0.1, 0.1);
    return obj;
}

// Create a custom particle at a given position, element, and velocity
GravitationalObject* createParticleAt(Vector3* pos, enum element element, Vector3* velocity) {
    GravitationalObject* obj = malloc(sizeof(GravitationalObject));
    obj->name = "Custom";
    obj->element = element;
    obj->position = *pos;
    obj->force = (Vector3){0, 0, 0};
    obj->velocity = *velocity;
    return obj;
}

// Add multiple random objects to the object list within a given room size
void randomObjectsFor(int count, ObjectList* objList, Vector3 room) {
    for(int i = 0; i < count; i++) {
        Vector3 pos = {GetRandomValue(room.x*-1, room.x), GetRandomValue(room.x*-1, room.x), GetRandomValue(room.x*-1, room.x)};
        GravitationalObject* obj = createRandomParticleAt(&pos);
        addObjectList(obj, objList);
    }
}

// Remove an object at a specific index from the object list
void removeObjectAtIndex(ObjectList* list, int index) {
    if (index < 0 || index >= list->size) return;
    if (list->gObjs[index] != NULL) {
        free(list->gObjs[index]);
    }
    for (int i = index; i < list->size - 1; i++) {
        list->gObjs[i] = list->gObjs[i + 1];
    }
    list->size--;
    if (list->size > 0) {
        GravitationalObject** newArray = realloc(list->gObjs, list->size * sizeof(GravitationalObject*));
        if (newArray != NULL) {
            list->gObjs = newArray;
        }
    } else {
        free(list->gObjs);
        list->gObjs = NULL;
    }
}

// Detect and handle collisions between objects using spatial hashing
void CalculateCollision(ObjectList* list) {
    SpatialHash grid = {0};
    float cellSize = 2.f;
    // Insert all objects into the grid
    for (int i = 0; i < list->size; i++) {
        insertObject(&grid, list->gObjs[i], cellSize);
    }
    // Check for collisions in each cell and neighbors
    for (int h = 0; h < HASH_SIZE; h++) {
        CellEntry* entry = grid.table[h];
        while (entry) {
            GravitationalObject* a = entry->obj;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        unsigned int nh = hashCell(
                            (int)floor(a->position.x / cellSize) + dx,
                            (int)floor(a->position.y / cellSize) + dy,
                            (int)floor(a->position.z / cellSize) + dz
                        );
                        CellEntry* neighbor = grid.table[nh];
                        while (neighbor) {
                            GravitationalObject* b = neighbor->obj;
                            if (a == b) { neighbor = neighbor->next; continue; }
                            float dx = a->position.x - b->position.x;
                            float dy = a->position.y - b->position.y;
                            float dz = a->position.z - b->position.z;
                            float distSq = dx*dx + dy*dy + dz*dz;
                            if (distSq <= PARTICLERADIUS*PARTICLERADIUS) {
                                // Collision response can be implemented here
                            }
                            neighbor = neighbor->next;
                        }
                    }
                }
            }
            entry = entry->next;
        }
    }
    freeSpatialHash(&grid);
}

// Handle user input for creating new objects
void handleInput(ObjectList* objectList, Camera3D* camera) {
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        Vector3 pos = GetMouseWorldPoint(camera, 100);
        GravitationalObject* newObj = createRandomParticleAt(&pos);
        addObjectList(newObj, objectList);
    }
    // Additional input handling for custom object creation can be added here
}


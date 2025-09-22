#include "Calculations.h"

#define HASH_SIZE 10007


const float G = 6.67430e-11f; // Universal gravitational constant

static int gUseGPU = 1;         // default: try GPU
static int gCullingEnabled = 1; // default: culling on

void SetUseGPU(int enabled) { gUseGPU = enabled ? 1 : 0; }
int IsUseGPU(void) { return gUseGPU; }
void SetCullingEnabled(int enabled) { gCullingEnabled = enabled ? 1 : 0; }
int IsCullingEnabled(void) { return gCullingEnabled; }

// Use the compute shader to calculate gravity for all objects
void ComputeGravitationWithShader(ObjectList* oList, float deltaTime) {
    if (oList->size == 0) return;
    if (!gUseGPU) {
        CalculateGravitation(oList);
        MoveParticles(oList, deltaTime);
        return;
    }
    // --- New grid-based GPU path ---
    float cellSize = 20.f;
    Grid* grid = getGrid(oList, cellSize);
    if (!grid) {
        CalculateGravitation(oList);
        MoveParticles(oList, deltaTime);
        return;
    }
    int numObjects = oList->size;
    int numCells = (int)(grid->gridSize.x * grid->gridSize.y * grid->gridSize.z);
    // Prepare GPUObject array
    GPUObject* gpuObjs = malloc(sizeof(GPUObject) * numObjects);
    for (int i = 0; i < numObjects; i++) {
        GravitationalObject* obj = oList->gObjs[i];
        gpuObjs[i].position[0] = obj->position.x;
        gpuObjs[i].position[1] = obj->position.y;
        gpuObjs[i].position[2] = obj->position.z;
        gpuObjs[i].velocity[0] = obj->velocity.x;
        gpuObjs[i].velocity[1] = obj->velocity.y;
        gpuObjs[i].velocity[2] = obj->velocity.z;
        gpuObjs[i].mass = (float)obj->element;
    }
    // Prepare GPUGridCell and object index arrays
    extern void flattenGridForGPU(const Grid* grid, GPUGridCell** outCells, int* outCellCount, unsigned int** outObjIndices, int* outObjIndexCount, ObjectList* objList);
    GPUGridCell* gpuCells = NULL;
    unsigned int* objIndices = NULL;
    int objIndexCount = 0, cellCount = 0;
    flattenGridForGPU(grid, &gpuCells, &cellCount, &objIndices, &objIndexCount, oList);
    // Call the new computeGravity
    int ok = computeGravity(
        gpuObjs, numObjects,
        gpuCells, cellCount,
        objIndices, objIndexCount,
        grid->gridSize, cellSize, deltaTime, G
    );
    if (!ok) {
        if (DEBUG_MODE) printf("[ComputeGravitationWithShader] Falling back to CPU path.\n");
        CalculateGravitation(oList);
        MoveParticles(oList, deltaTime);
        free(gpuObjs);
        free(gpuCells);
        free(objIndices);
        freeGrid(grid);
        return;
    }
    // Copy results back to GravitationalObject
    for (int i = 0; i < numObjects; i++) {
        GravitationalObject* obj = oList->gObjs[i];
        obj->position.x = gpuObjs[i].position[0];
        obj->position.y = gpuObjs[i].position[1];
        obj->position.z = gpuObjs[i].position[2];
        obj->velocity.x = gpuObjs[i].velocity[0];
        obj->velocity.y = gpuObjs[i].velocity[1];
        obj->velocity.z = gpuObjs[i].velocity[2];
    }
    free(gpuObjs);
    free(gpuCells);
    free(objIndices);
    freeGrid(grid);
}

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

// Detect and handle collisions between objects using spatial hashing
void CalculateCollision(ObjectList* list, int particleRadius) {
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
                            if (distSq <= particleRadius*particleRadius) {
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
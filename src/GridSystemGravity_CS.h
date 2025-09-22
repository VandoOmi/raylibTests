#ifndef GS_GRAVITY_CS_H
#define GS_GRAVITY_CS_H

#define NOGDI
#define NOUSER
#include <GL/gl3w.h>
#include "settings.h"

typedef struct GPUGridCell {
    float center[3];      // Center of mass of the cell
    float mass;           // Total mass in the cell
    unsigned int objectStart; // Start index in the flat object index array
    unsigned int objectCount; // Number of objects in this cell
    unsigned int _pad[2];     // Padding for 16-byte alignment (std430)
} GPUGridCell;

// Flattens the grid for GPU upload. Allocates arrays, returns their sizes.
void flattenGridForGPU(const Grid* grid, GPUGridCell** outCells, int* outCellCount, unsigned int** outObjIndices, int* outObjIndexCount, ObjectList* objList);

GLuint createGravityComputeShader();
int computeGravity(
    GPUObject* objects, int numObjects,
    GPUGridCell* cells, int numCells,
    unsigned int* objIndices, int numObjIndices,
    Vector3 gridSize, float cellSize, float deltatime, float G
);

#endif
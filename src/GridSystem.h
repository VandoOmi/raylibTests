#ifndef GRID_SYSTEM_H
#define GRID_SYSTEM_H


#include <Raylib.h>

// Forward declarations to avoid circular dependency
typedef struct GravitationalObject GravitationalObject;
typedef struct ObjectList ObjectList;

typedef struct Cell {
    float mass;
    Vector3 center;
    GravitationalObject** objects; // Array of pointers to objects in this cell
    int objectCount;
    int objectCapacity;
} Cell;

typedef struct Grid 
{
    Cell* cells; 
    Vector3 gridSize;
    float cellSize;
} Grid;

Grid* getGrid(ObjectList* objList, float cellSize);
void updateGrid(Grid* grid, ObjectList* objList);
void freeGrid(Grid* grid);

#endif
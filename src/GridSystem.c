#include "GridSystem.h"


Grid* getGrid(ObjectList* objList, float cellSize) {
	if (!objList || objList->size == 0) return NULL;

	// Find bounds
	Vector3 min = objList->gObjs[0]->position;
	Vector3 max = objList->gObjs[0]->position;
	for (int i = 1; i < objList->size; i++) {
		Vector3 p = objList->gObjs[i]->position;
		if (p.x < min.x) min.x = p.x;
		if (p.y < min.y) min.y = p.y;
		if (p.z < min.z) min.z = p.z;
		if (p.x > max.x) max.x = p.x;
		if (p.y > max.y) max.y = p.y;
		if (p.z > max.z) max.z = p.z;
	}

	int nx = (int)((max.x - min.x) / cellSize + 1);
	int ny = (int)((max.y - min.y) / cellSize + 1);
	int nz = (int)((max.z - min.z) / cellSize + 1);

	// Allocate grid and a count array for averaging
	Grid* grid = (Grid*)malloc(sizeof(Grid));
	grid->gridSize = (Vector3){nx, ny, nz};
	grid->cellSize = cellSize;
	int cellCount = nx * ny * nz;
	grid->cells = (Cell*)calloc(cellCount, sizeof(Cell));
	int* counts = (int*)calloc(cellCount, sizeof(int));
	// Initialize object arrays for each cell
	for (int i = 0; i < cellCount; i++) {
		grid->cells[i].objects = NULL;
		grid->cells[i].objectCount = 0;
		grid->cells[i].objectCapacity = 0;
	}

	// Place objects in grid, accumulate mass and position
	for (int i = 0; i < objList->size; i++) {
		GravitationalObject* obj = objList->gObjs[i];
		int x = (int)((obj->position.x - min.x) / cellSize);
		int y = (int)((obj->position.y - min.y) / cellSize);
		int z = (int)((obj->position.z - min.z) / cellSize);
		int idx = x + nx * (y + ny * z);
		Cell* cell = &grid->cells[idx];
		cell->mass += obj->element;
		cell->center.x += obj->position.x;
		cell->center.y += obj->position.y;
		cell->center.z += obj->position.z;
		counts[idx]++;
		// Add object pointer to cell's object array
		if (cell->objectCount >= cell->objectCapacity) {
			int newCap = cell->objectCapacity == 0 ? 4 : cell->objectCapacity * 2;
			cell->objects = (GravitationalObject**)realloc(cell->objects, newCap * sizeof(GravitationalObject*));
			cell->objectCapacity = newCap;
		}
		cell->objects[cell->objectCount++] = obj;
	}

	// Average the center for each cell
	for (int i = 0; i < cellCount; i++) {
		if (counts[i] > 0) {
			grid->cells[i].center.x /= counts[i];
			grid->cells[i].center.y /= counts[i];
			grid->cells[i].center.z /= counts[i];
		}
	}
	free(counts);

	return grid;
}

void updateGrid(Grid* grid, ObjectList* objList) {
    float cellSize = grid->cellSize;
	if (grid) {
		freeGrid(grid);
		grid = NULL;
	}
	grid = getGrid(objList, cellSize);
}

void freeGrid(Grid* grid) {
	if (!grid) return;
	int cellCount = (int)(grid->gridSize.x * grid->gridSize.y * grid->gridSize.z);
	for (int i = 0; i < cellCount; i++) {
		free(grid->cells[i].objects);
	}
	free(grid->cells);
	free(grid);
}

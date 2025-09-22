
#include "particle.h"


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


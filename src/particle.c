#include "particle.h"

const float G = 6.67430e-11f;
const int PARTICLERADIUS = 1; // in km
#define HASH_SIZE 10007

Vector3 firstPos = { 0, 0, 0 };

enum element{
    hydrogen = 37659,
    helium = 74564,
    oxygen = 598608,
    carbon = 949646300,
    neon = 376968,
    iron = 3298418600
};

typedef struct GravitationalObject
{
    const char* name;
    enum element element;
    Vector3 position;
    Vector3 force;
    Vector3 velocity;
} GravitationalObject;

typedef struct CellEntry {
    GravitationalObject* obj;
    struct CellEntry* next;
} CellEntry;

typedef struct {
    CellEntry* table[HASH_SIZE];
} SpatialHash;

// Hashfunktion für 3D-Zellenkoordinaten
unsigned int hashCell(int x, int y, int z) {
    unsigned int h = 73856093u * x ^ 19349663u * y ^ 83492791u * z;
    return h % HASH_SIZE;
}

// Objekt einer Zelle hinzufügen
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

void freeSpatialHash(SpatialHash* grid) {
    for (int h = 0; h < HASH_SIZE; h++) {
        CellEntry* entry = grid->table[h];
        while (entry) {
            CellEntry* next = entry->next;
            free(entry);
            entry = next;
        }
        grid->table[h] = NULL; // optional
    }
}

typedef struct ObjectList
{
    GravitationalObject** gObjs;
    int size;
} ObjectList;

Color getColor(enum element element) {
    Color color;
    switch (element)
    {
        case hydrogen:
            return RAYWHITE;
        case helium:
            return RED;
        case oxygen:
            return BLUE;
        case carbon:
            return GRAY;
        case neon:             //////////////////////////////////////////////////////////////////////////////zu masse ändern;
            return PINK;
        case iron:
            return LIGHTGRAY;
    }
}

void drawParticle(GravitationalObject *obj) {

    DrawSphere((Vector3)obj->position, PARTICLERADIUS, getColor(obj->element));

    if (DEBUG_MODE) {
        printf("[DRAW] %s: pos=(%.2f, %.2f, %.2f)\n",
       obj->name, obj->position.x, obj->position.y, obj->position.z);
    }
}

void DrawParticles(ObjectList* oList) {
    for (int i = 0; i < oList->size; i++) {
        GravitationalObject *gObj = oList->gObjs[i];
        drawParticle(gObj);
    }
}

Vector3 GetMouseWorldPoint(const Camera3D *camera, float distance) {
    // Hole den Strahl von der Kamera durch den Mauszeiger
    Ray ray = GetMouseRay(GetMousePosition(), *camera);

    // Berechne den Punkt: Startposition + Richtung * distance
    Vector3 point = {
        ray.position.x + ray.direction.x * distance,
        ray.position.y + ray.direction.y * distance,
        ray.position.z + ray.direction.z * distance
    };

    return point;
}

void CalculateGravitation(ObjectList* oList) {
    for (int i = 0; i < oList->size; i++) {
        oList->gObjs[i]->force.x = 0.0f;
        oList->gObjs[i]->force.y = 0.0f;
        oList->gObjs[i]->force.z = 0.0f;
    }

    float cellSize = 20.f;

    SpatialHash grid = {0};
    for (int i = 0; i < oList->size; i++) {
        insertObject(&grid, oList->gObjs[i], cellSize);
    }

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
                            if (a == b) { 
                                neighbor = neighbor->next; 
                                continue; 
                            }

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

void MoveParticles(ObjectList* oList, float deltaTime) {
    for (int i = 0; i < oList->size; i++) {
        moveObject(oList->gObjs[i], deltaTime);
    }
}

ObjectList* createObjectList() {
    ObjectList* list = malloc(sizeof(ObjectList));
    list->gObjs = NULL;
    list->size = 0;
    return list;
}

void addObjectList(GravitationalObject* obj, ObjectList* oList) {
    GravitationalObject** temp_array = realloc(oList->gObjs, (oList->size+1) * sizeof(GravitationalObject*));
    
    if (temp_array == NULL) {
        fprintf(stderr, "[ERROR] Speicher konnte nicht erweitert werden.\n");
        return;
    }

    
    oList->gObjs = temp_array;
    oList->gObjs[oList->size] = obj;
    oList->size++;
}

void freeObjectList(ObjectList* oList) {
    for (int i = 0; i < oList->size; i++) {
        free(oList->gObjs[i]);
    }
    free(oList->gObjs);
    free(oList);  
    
}

GravitationalObject* createRandomParticleAt(Vector3* pos) {
    GravitationalObject* obj = malloc(sizeof(GravitationalObject));

    enum element elements[] = {hydrogen, helium, oxygen, carbon, neon, iron};

    obj->name = "Random";
    obj->element = elements[rand() % 6];
    obj->position = *pos;
    obj->force.x = 0;
    obj->force.y = 0;
    obj->force.z = 0;
    obj->velocity.x = GetRandomValue(-0.1, 0.1);  
    obj->velocity.y = GetRandomValue(-0.1, 0.1); 
    obj->velocity.z = GetRandomValue(-0.1, 0.1);

    return obj;
}

GravitationalObject* createParticleAt(Vector3* pos, enum element element, Vector3* velocity) {
    GravitationalObject* obj = malloc(sizeof(GravitationalObject));

    obj->name = "Custom";
    obj->element = element;
    obj->position = *pos;
    obj->force.x = 0;
    obj->force.y = 0;
    obj->force.z = 0;
    obj->velocity = *velocity;

    return obj;
}

void randomObjectsFor(int count, ObjectList* objList, Vector3 room) {
    for(int i = 0; i < count; i++) {
        Vector3 pos = {GetRandomValue(room.x*-1, room.x), GetRandomValue(room.x*-1, room.x), GetRandomValue(room.x*-1, room.x)};
        GravitationalObject* obj = createRandomParticleAt(&pos);
        addObjectList(obj, objList);
    }
}

void removeObjectAtIndex(ObjectList* list, int index) {
    if (index < 0 || index >= list->size) return;

    // Achtung: Nur free(), wenn das Objekt auch dynamisch (per malloc) erstellt wurde!
    if (list->gObjs[index] != NULL) {
        free(list->gObjs[index]);
    }

    // Nachrücken
    for (int i = index; i < list->size - 1; i++) {
        list->gObjs[i] = list->gObjs[i + 1];
    }

    list->size--;

    // realloc nur, wenn size > 0
    if (list->size > 0) {
        GravitationalObject** newArray = realloc(list->gObjs, list->size * sizeof(GravitationalObject*));
        if (newArray != NULL) {
            list->gObjs = newArray;
        }
    } else {
        free(list->gObjs);     // alle Elemente wurden entfernt
        list->gObjs = NULL;
    }
}

void CalculateCollision(ObjectList* list) {
    SpatialHash grid = {0};
    float cellSize = 2.f;

    // 1. Objekte ins Grid einfügen
    for (int i = 0; i < list->size; i++) {
        insertObject(&grid, list->gObjs[i], cellSize);
    }

    // 2. Kollisionsprüfungen
    for (int h = 0; h < HASH_SIZE; h++) {
        CellEntry* entry = grid.table[h];

        while (entry) {
            GravitationalObject* a = entry->obj;

            // Nachbarn in dieser und angrenzenden Zellen prüfen
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
                            if (a == b) { 
                                neighbor = neighbor->next; 
                                continue; 
                            }

                            // Kollisionscheck (wie in deinem Code, mit Distanz²)
                            float dx = a->position.x - b->position.x;
                            float dy = a->position.y - b->position.y;
                            float dz = a->position.z - b->position.z;
                            float distSq = dx*dx + dy*dy + dz*dz;

                            if (distSq <= PARTICLERADIUS*PARTICLERADIUS) {
                                // Kollisionsreaktion (dein Code hier)
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
/*
int makeInfoLabel(Rectangle* panel, int lineHeight, int length, const char* text) {
    Rectangle infoLabelBounds = { 
        panel->x + ((panel->width-length)/2), 
        lineHeight, 
        length, 
        10 
    };
    GuiLabel(infoLabelBounds, text);
    
    return lineHeight + infoLabelBounds.height;
}

int makeLabelAndSlider(Rectangle* panel, int lineHeight, const char* labelText, float* value, float minValue, float maxValue) {
    int optionsPaddingLeft = 10;

    Rectangle labelBounds = { 
        panel->x + optionsPaddingLeft, 
        lineHeight, 
        75, 
        10 
    };
    GuiLabel(labelBounds, labelText);
    
    Rectangle countBounds = { 
        panel->x + optionsPaddingLeft + labelBounds.width, 
        lineHeight, 
        50, 
        10 
    };
    GuiLabel(countBounds, TextFormat("%.0f", *value));
    
    Rectangle sliderBounds = { 
        labelBounds.x + labelBounds.width + countBounds.width + 25, 
        lineHeight, 
        100, 
        10 
    };
    GuiSlider(sliderBounds, TextFormat("%.0f", minValue), TextFormat("%.0f", maxValue), value, minValue, maxValue);

    return lineHeight + sliderBounds.height;
}

int makeHeaderLabel(Rectangle* panel, int lineHeight, int length, const char* text) {
    Rectangle headerBounds = { 
        panel->x + ((panel->width-length)/2), 
        lineHeight, 
        length, 
        20 
    };
    GuiLabel(headerBounds, text);
    
    return lineHeight + headerBounds.height;
}

void handleGUI(ObjectList* objectList) {

    //Background
    Rectangle panel = { 10, 10, 300, 190};

    DrawRectangleRounded(panel, 0.1f, 100, Fade(WHITE, 0.1f));
    DrawRectangleRoundedLines(panel, 0.1f, 100, Fade(WHITE, 0.4f));

    //Foreground
    int optionsPaddingLeft = 10;

    int paddingTop = 20;
    int lineHeight = panel.y + paddingTop;

    paddingTop = 20;
    lineHeight = makeLabelAndSlider
    (
        &panel, lineHeight, "graviation: ", &G, 0.0f, 0.000001f
    )+ paddingTop;
    
    int createLength = 74;
    paddingTop = 0;
    lineHeight = makeHeaderLabel
    (
        &panel, lineHeight, createLength, "-- CREATE --"
    ) + paddingTop;

    int infoLength = 200;
    paddingTop = 20;
    lineHeight = makeInfoLabel
    (
        &panel, lineHeight, infoLength, "Right-Click to create a random object."
    ) + paddingTop;

    static float massValue = 50.0f;
    static float radiusValue = 10.0f;
    paddingTop = 10;

    lineHeight = makeLabelAndSlider
    (
        &panel, lineHeight, "mass: ", &massValue, 0.0f, 1000.0f
    ) + paddingTop;

    paddingTop = 20;
    lineHeight = makeLabelAndSlider
    (
        &panel, lineHeight, "radius: ", &radiusValue, 0.0f, 100.0f
    ) + paddingTop;

    paddingTop = 5;
    infoLength = 120;
    lineHeight = makeInfoLabel
    (
        &panel, lineHeight, infoLength, "[LEFT-CLICK + SHIFT]"
    ) + paddingTop;
    infoLength = 125;
    lineHeight = makeInfoLabel
    (
        &panel, lineHeight, infoLength, "Create a custom object."
    ) + paddingTop;
}
*/
void handleInput(ObjectList* objectList, Camera3D* camera) {
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        Vector3 pos = GetMouseWorldPoint(camera, 100);
        GravitationalObject* newObj = createRandomParticleAt(&pos);
        addObjectList(newObj, objectList);
    }

    /*if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsKeyDown(KEY_LEFT_SHIFT))
    {
        firstPos.x = GetMousePosition().x;
        firstPos.y = GetMousePosition().y;

        if (DEBUG_MODE) 
        {
            printf("[CLICK] Erste Position gesetzt bei %.2f, %.2f, %.2f\n", firstPos.x, firstPos.y, firstPos.z);
        }
    }
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && IsKeyDown(KEY_LEFT_SHIFT))  
    {
        Vector2 secondPos = GetMousePosition();
        Vector3 velocity = { (secondPos.x - firstPos.x), (secondPos.y - firstPos.y) , 0. };
        GravitationalObject* newObj = createParticleAt(&firstPos, massValue, radiusValue, &velocity);
        addObjectList(newObj, objectList);

        if (DEBUG_MODE) 
        {
            printf("[CLICK] Neues Objekt erstellt bei %.2f, %.2f mit Masse %.2f und Radius %.2f\n", 
                firstPos.x, firstPos.y, newObj->mass, newObj->radius);
        }
    }*/
}


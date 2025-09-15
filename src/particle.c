#include <raylib.h> 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>


#define DEBUG_MODE 0
#ifndef PI
#define PI 3.14159265358979323846
#endif

float G = 0.00000000006674f;
float massValue = 50.0f;
float radiusValue = 10.0f;
int particleRadius = 1; // in km
Vector3 firstPos = { 0, 0, 0 };
float volume = 0.;

enum element{
    hydrogen = 8990,
    helium = 17800,
    oxygen = 142900,
    carbon = 226700000,
    neon = 89990,
    iron = 787400000
};

typedef struct gravitationalObject
{
    const char* name;
    enum element element;
    Vector3 position;
    Vector3 force;
    Vector3 velocity;
} GravitationalObject;

typedef struct gList
{
    GravitationalObject** gObjs;
    int size;
} ObjectList;

Color getColor(enum element element) {
    Color color;
    switch (element)
    {
        case 8990:
            return RAYWHITE;
        case 17800:
            return RED;
        case 142900:
            return BLUE;
        case 226700000:
            return GRAY;
        case 89990:
            return PINK;
        case 787400000:
            return LIGHTGRAY;
    }
}

void drawParticle(GravitationalObject *obj) {

    DrawSphere((Vector3)obj->position, particleRadius, getColor(obj->element));

    if (DEBUG_MODE) {
        printf("[DRAW] %s: pos=(%.2f, %.2f, %.2f)\n",
       obj->name, obj->position.x, obj->position.y, obj->position.z);
    }
}

void drawParticles(ObjectList* oList) {
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

void calcGravitation(ObjectList* oList) {
    for (int i = 0; i < oList->size; i++) {
        oList->gObjs[i]->force.x = 0.0f;
        oList->gObjs[i]->force.y = 0.0f;
        oList->gObjs[i]->force.z = 0.0f;
    }

    if (DEBUG_MODE) {
        printf("[CALC] Kräfte zurückgesetzt.\n");
    }

    for (int i = 0; i < oList->size; i++) {
        for (int j = i + 1; j < oList->size; j++) {
            float dX = oList->gObjs[j]->position.x - oList->gObjs[i]->position.x;
            float dY = oList->gObjs[j]->position.y - oList->gObjs[i]->position.y;
            float dZ = oList->gObjs[j]->position.z - oList->gObjs[i]->position.z;
            float r = sqrt(dX * dX + dY * dY + dZ * dZ);

            float volume = ((4.f/3.f)*PI*particleRadius*particleRadius*particleRadius);

            if (r > 1e-10f) {
                float m1 = volume * oList->gObjs[i]->element;
                float m2 = volume * oList->gObjs[j]->element;
                float f = (G * m1 * m2) / (r * r);

                float forceX = f * (dX / r);
                float forceY = f * (dY / r);
                float forceZ = f * (dZ / r);

                oList->gObjs[i]->force.x += forceX;
                oList->gObjs[i]->force.y += forceY;
                oList->gObjs[i]->force.z += forceZ;

                oList->gObjs[j]->force.x -= forceX;
                oList->gObjs[j]->force.y -= forceY;
                oList->gObjs[j]->force.z -= forceZ;

                if (DEBUG_MODE) {
                    printf("[CALC] Gravitation %s <-> %s: Fx=%.4f, Fy=%.4f, Fz=%.4f, Abstand=%.2f\n", oList->gObjs[i]->name, oList->gObjs[j]->name, forceX, forceY, forceZ, r);
                }
            }
        }
    }
}

void moveObject(GravitationalObject *obj, float deltaTime) {
    float aX = obj->force.x/(obj->element * volume);
    float aY = obj->force.y/(obj->element * volume);
    float aZ = obj->force.z/(obj->element * volume);

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

void moveObjects(ObjectList* oList, float deltaTime) {
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

void handleCollisions(ObjectList* list) {
    for (int i = 0; i < list->size; i++) {
        GravitationalObject* a = list->gObjs[i];

        for (int j = i + 1; j < list->size; j++) {
            GravitationalObject* b = list->gObjs[j];

            float dx = a->position.x - b->position.x;
            float dy = a->position.y - b->position.y;
            float dz = a->position.z - b->position.z;
            float distance = sqrtf(dx*dx + dy*dy + dz*dz);
            if (DEBUG_MODE) {
                    printf("Checking collision between '%s' and '%s': distance = %.2f",
                    a->name, b->name, distance);
                }
            


            if (distance <= particleRadius*2) {

                Vector3 delta = { b->position.x - a->position.x,
                  b->position.y - a->position.y,
                  b->position.z - a->position.z };

                float dist = sqrtf(delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);
                if (dist == 0.0f) dist = 1e-6f; // numerische Sicherheit

                // Normalisierte Stoßrichtung
                Vector3 n = { delta.x / dist, delta.y / dist, delta.z / dist };

                // Relative Geschwindigkeit
                Vector3 vRel = { a->velocity.x - b->velocity.x,
                                a->velocity.y - b->velocity.y,
                                a->velocity.z - b->velocity.z };

                // Geschwindigkeit entlang der Stoßrichtung
                float vDot = vRel.x*n.x + vRel.y*n.y + vRel.z*n.z;
                if (vDot > 0) continue; // Objekte entfernen sich bereits

                // Impuls ändern (vereinfachte 1:1 Masse)
                float impulse = -2.0f * vDot / 2.0f;
                a->velocity.x += impulse * n.x;
                a->velocity.y += impulse * n.y;
                a->velocity.z += impulse * n.z;
                b->velocity.x -= impulse * n.x;
                b->velocity.y -= impulse * n.y;
                b->velocity.z -= impulse * n.z;

                // Position minimal verschieben, damit sie sich nicht überlappen
                float overlap = 2*particleRadius - dist;
                a->position.x -= n.x * overlap / 2;
                a->position.y -= n.y * overlap / 2;
                a->position.z -= n.z * overlap / 2;
                b->position.x += n.x * overlap / 2;
                b->position.y += n.y * overlap / 2;
                b->position.z += n.z * overlap / 2;
            }
        }
    }
}

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

int main(){
    const int windowSizeX = 1960;
    const int windowSizeY = 1080;
    volume = ((4/3)*PI*particleRadius*particleRadius);


    InitWindow(windowSizeX, windowSizeY, "Gravitations-Simulation");
    SetWindowState(FLAG_FULLSCREEN_MODE);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 100.0f, 100.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;



    ObjectList* objectList = createObjectList();

    //loop
    float t_delta = 0;
    float t_tick = 1.0f / 170.0f;
    float t_temp = 0;

    while(!WindowShouldClose()){
        
        t_delta = GetFrameTime();
        t_temp += t_delta;

        if (DEBUG_MODE) {
            printf("---- Frame Start | deltaTime = %.5f ----\n", t_delta);
        }
        UpdateCamera(&camera, CAMERA_FREE);

        handleInput(objectList, &camera);

        while (t_temp >= t_tick) {
            calcGravitation(objectList);
            moveObjects(objectList, t_tick);
            handleCollisions(objectList);
            
            t_temp -= t_tick;
        }
        
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                DrawGrid(200, 10.0f);
                drawParticles(objectList);
            EndMode3D();
            handleGUI(objectList);
        EndDrawing();

        if (DEBUG_MODE) {
            printf("---- Frame End --------------------------\n\n");
        }
       
    }

    //end
    CloseWindow();

    freeObjectList(objectList);

    return 0;
}


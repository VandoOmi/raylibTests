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

float G = 0.0f;
float massValue = 50.0f;
float radiusValue = 10.0f;
Vector3 firstPos = { 0, 0, 0 };

typedef struct gravitationalObject
{
    const char* name;
    float mass;
    Vector3 position;
    float radius;
    Color color;
    Vector3 force;
    Vector3 velocity;
} GravitationalObject;

typedef struct gList
{
    GravitationalObject** gObjs;
    int size;
} ObjectList;



void drawGravitationalObject(GravitationalObject *obj) {
    DrawSphere((Vector3)obj->position, obj->radius, obj->color);

    if (DEBUG_MODE) {
        printf("[DRAW] %s: pos=(%.2f, %.2f, %.2f)\n",
       obj->name, obj->position.x, obj->position.y, obj->position.z);
    }
}

void drawGravitationalObjectArray(ObjectList* oList) {
    for (int i = 0; i < oList->size; i++) {
        GravitationalObject *gObj = oList->gObjs[i];
        drawGravitationalObject(gObj);
    }
}

bool rayIntersectPlaneY(float y, Ray ray, Vector3 *hit)
{
    if (fabsf(ray.direction.y) < 1e-6f) return false; // Strahl ist fast parallel
    float t = (y - ray.position.y) / ray.direction.y;
    if (t <= 0.0f) return false; // Schnittpunkt liegt hinter der Kamera
    hit->x = ray.position.x + ray.direction.x * t;
    hit->y = y;
    hit->z = ray.position.z + ray.direction.z * t;
    return true;
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

            if (r != 0) {
                float f = (G * oList->gObjs[i]->mass * oList->gObjs[j]->mass) / (r * r);
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
    float aX = obj->force.x/obj->mass;
    float aY = obj->force.y/obj->mass;
    float aZ = obj->force.z/obj->mass;

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

GravitationalObject* createRandomObjectAt(Vector3* pos) {
    GravitationalObject* obj = malloc(sizeof(GravitationalObject));

    obj->name = "Random";
    obj->mass = GetRandomValue(1, 100);  
    obj->radius = 3 + (obj->mass / 20.0f);  
    obj->color = (Color){ GetRandomValue(100,255), GetRandomValue(100,255), GetRandomValue(100,255), 255 };
    obj->position = *pos;
    obj->force.x = 0;
    obj->force.y = 0;
    obj->force.z = 0;
    obj->velocity.x = GetRandomValue(-10, 10);  
    obj->velocity.y = GetRandomValue(-10, 10); 
    obj->velocity.z = GetRandomValue(-10, 10);

    return obj;
}

GravitationalObject* createObjectAt(Vector3* pos, float mass, float radius, Vector3* velocity) {
    GravitationalObject* obj = malloc(sizeof(GravitationalObject));

    obj->name = "Custom";
    obj->mass = mass;  
    obj->radius = radius;  
    obj->color = (Color){ GetRandomValue(100,255), GetRandomValue(100,255), GetRandomValue(100,255), 255 };
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
                    printf("Checking collision between '%s' and '%s': distance = %.2f, sum of radii = %.2f\n",
                    a->name, b->name, distance, a->radius + b->radius);
                }
            


            if (distance < a->radius + b->radius) {
                // Kollision erkannt – neues Objekt erstellen
                GravitationalObject* merged = malloc(sizeof(GravitationalObject));

                merged->mass = a->mass + b->mass;

                // Schwerpunkt
                merged->position.x = (a->position.x * a->mass + b->position.x * b->mass) / merged->mass;
                merged->position.y = (a->position.y * a->mass + b->position.y * b->mass) / merged->mass;
                merged->position.z = (a->position.z * a->mass + b->position.z * b->mass) / merged->mass;

                // Impulserhaltung (vereinfacht)
                merged->velocity.x = (a->velocity.x * a->mass + b->velocity.x * b->mass) / merged->mass;
                merged->velocity.y = (a->velocity.y * a->mass + b->velocity.y * b->mass) / merged->mass;
                merged->velocity.z = (a->velocity.z * a->mass + b->velocity.z * b->mass) / merged->mass;
                
                // Radius proportional zur Wurzel der Masse (optional)
                float flaecheA = PI * (a->radius * a->radius);
                float flaecheB = PI * (b->radius * b->radius);
                merged->radius = sqrtf((flaecheA+flaecheB)/PI);
                // Farbe: die des größeren Objekts
                merged->color = (a->mass > b->mass) ? a->color : b->color;

                merged->force.x = 0;
                merged->force.y = 0;
                merged->force.z = 0;
                merged->name = "merged";

                // Alte Objekte entfernen
                removeObjectAtIndex(list, j); // erst j!
                removeObjectAtIndex(list, i);
                addObjectList(merged, list);

                // Restart wegen geänderter Liste
                return handleCollisions(list);
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
        &panel, lineHeight, "graviation: ", &G, 0.0f, 200.0f
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
        Ray ray = GetMouseRay(GetMousePosition(), *camera);
        Vector3 pos;
        if (rayIntersectPlaneY(0.0f, ray, &pos)) {
            GravitationalObject* newObj = createRandomObjectAt(&pos);
            addObjectList(newObj, objectList);
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsKeyDown(KEY_LEFT_SHIFT))
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
        GravitationalObject* newObj = createObjectAt(&firstPos, massValue, radiusValue, &velocity);
        addObjectList(newObj, objectList);

        if (DEBUG_MODE) 
        {
            printf("[CLICK] Neues Objekt erstellt bei %.2f, %.2f mit Masse %.2f und Radius %.2f\n", 
                firstPos.x, firstPos.y, newObj->mass, newObj->radius);
        }
    }
}

int main(){
    const int windowSizeX = 1200;
    const int windowSizeY = 1000;


    InitWindow(windowSizeX, windowSizeY, "Gravitations-Simulation");
    SetWindowState(FLAG_FULLSCREEN_MODE);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 1000.0f, 1000.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 0.0f, 0.0f };
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
        UpdateCamera(&camera, CAMERA_ORBITAL);

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
                drawGravitationalObjectArray(objectList);
                DrawGrid(20, 1.0f);
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


#include <raylib.h> 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>


#define DEBUG_MODE 0
#define EXAMPELS 0
#ifndef PI
#define PI 3.14159265358979323846
#endif

float G = 0.0f;
float massValue = 50.0f;
float radiusValue = 10.0f;
Vector2 firstPos = { 0, 0 };

typedef struct gravitationalObject
{
    const char* name;
    float mass;
    float posX;
    float posY;
    float radius;
    Color color;
    float forceX;
    float forceY;
    float velX;
    float velY;
} GravitationalObject;

typedef struct gList
{
    GravitationalObject** gObjs;
    int size;
} ObjectList;



void drawParticle(GravitationalObject *obj) {
    DrawCircle((int)obj->posX, (int)obj->posY, obj->radius, obj->color);

    if (DEBUG_MODE) {
        printf("[DRAW] %s: posX=%.2f, posY=%.2f\n", obj->name, obj->posX, obj->posY);
    }
}

void DrawParticles(ObjectList* oList) {
    for (int i = 0; i < oList->size; i++) {
        GravitationalObject *gObj = oList->gObjs[i];
        drawParticle(gObj);
    }
}

void CalculateGravitation(ObjectList* oList) {
    for (int i = 0; i < oList->size; i++) {
        oList->gObjs[i]->forceX = 0.0f;
        oList->gObjs[i]->forceY = 0.0f;
    }

    if (DEBUG_MODE) {
        printf("[CALC] Kräfte zurückgesetzt.\n");
    }

    for (int i = 0; i < oList->size; i++) {
        for (int j = i + 1; j < oList->size; j++) {
            float dX = oList->gObjs[j]->posX - oList->gObjs[i]->posX;
            float dY = oList->gObjs[j]->posY - oList->gObjs[i]->posY;
            float r = sqrt(dX * dX + dY * dY);

            if (r != 0) {
                float f = (G * oList->gObjs[i]->mass * oList->gObjs[j]->mass) / (r * r);
                float forceX = f * (dX / r);
                float forceY = f * (dY / r);

                oList->gObjs[i]->forceX += forceX;
                oList->gObjs[i]->forceY += forceY;
                oList->gObjs[j]->forceX -= forceX;
                oList->gObjs[j]->forceY -= forceY;

                if (DEBUG_MODE) {
                    printf("[CALC] Gravitation %s <-> %s: Fx=%.4f, Fy=%.4f, Abstand=%.2f\n", oList->gObjs[i]->name, oList->gObjs[j]->name, forceX, forceY, r);
                }
            }
        }
    }
}

void moveObject(GravitationalObject *obj, float deltaTime) {
    float aX = obj->forceX/obj->mass;
    float aY = obj->forceY/obj->mass;

    obj->velX += aX * deltaTime;
    obj->velY += aY * deltaTime;

    obj->posX += obj->velX * deltaTime;
    obj->posY += obj->velY * deltaTime;

    if (DEBUG_MODE) {
        printf("[MOVE] %s:\n", obj->name);
        printf("       aX=%.5f, aY=%.5f\n", aX, aY);
        printf("       velX=%.5f, velY=%.5f\n", obj->velX, obj->velY);
        printf("       posX=%.2f, posY=%.2f\n", obj->posX, obj->posY);
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

GravitationalObject* createRandomObjectAt(float x, float y) {
    GravitationalObject* obj = malloc(sizeof(GravitationalObject));

    obj->name = "Random";
    obj->mass = GetRandomValue(1, 100);  
    obj->radius = 3 + (obj->mass / 20.0f);  
    obj->color = (Color){ GetRandomValue(100,255), GetRandomValue(100,255), GetRandomValue(100,255), 255 };
    obj->posX = x;
    obj->posY = y;
    obj->forceX = 0;
    obj->forceY = 0;
    obj->velX = GetRandomValue(-10, 10);  
    obj->velY = GetRandomValue(-10, 10);

    return obj;
}

GravitationalObject* createObjectAt(Vector2* pos, float mass, float radius, Vector2* velocity) {
    GravitationalObject* obj = malloc(sizeof(GravitationalObject));

    obj->name = "Custom";
    obj->mass = mass;  
    obj->radius = radius;  
    obj->color = (Color){ GetRandomValue(100,255), GetRandomValue(100,255), GetRandomValue(100,255), 255 };
    obj->posX = pos->x;
    obj->posY = pos->y;
    obj->forceX = 0;
    obj->forceY = 0;
    obj->velX = velocity->x;  
    obj->velY = velocity->y;

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

void CalculateCollision(ObjectList* list) {
    for (int i = 0; i < list->size; i++) {
        GravitationalObject* a = list->gObjs[i];

        for (int j = i + 1; j < list->size; j++) {
            GravitationalObject* b = list->gObjs[j];

            float dx = a->posX - b->posX;
            float dy = a->posY - b->posY;
            float distance = sqrtf(dx*dx + dy*dy);
            if (DEBUG_MODE) {
                    printf("Checking collision between '%s' and '%s': distance = %.2f, sum of radii = %.2f\n",
                    a->name, b->name, distance, a->radius + b->radius);
                }
            


            if (distance < a->radius + b->radius) {
                // Kollision erkannt – neues Objekt erstellen
                GravitationalObject* merged = malloc(sizeof(GravitationalObject));

                merged->mass = a->mass + b->mass;

                // Schwerpunkt
                merged->posX = (a->posX * a->mass + b->posX * b->mass) / merged->mass;
                merged->posY = (a->posY * a->mass + b->posY * b->mass) / merged->mass;

                // Impulserhaltung (vereinfacht)
                merged->velX = (a->velX * a->mass + b->velX * b->mass) / merged->mass;
                merged->velY = (a->velY * a->mass + b->velY * b->mass) / merged->mass;

                // Radius proportional zur Wurzel der Masse (optional)
                float flaecheA = PI * (a->radius * a->radius);
                float flaecheB = PI * (b->radius * b->radius);
                merged->radius = sqrtf((flaecheA+flaecheB)/PI);
                // Farbe: die des größeren Objekts
                merged->color = (a->mass > b->mass) ? a->color : b->color;

                merged->forceX = 0;
                merged->forceY = 0;
                merged->name = "merged";

                // Alte Objekte entfernen
                removeObjectAtIndex(list, j); // erst j!
                removeObjectAtIndex(list, i);
                addObjectList(merged, list);

                // Restart wegen geänderter Liste
                return CalculateCollision(list);
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

void handleInput(ObjectList* objectList) {
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) 
    {
        Vector2 mouse = GetMousePosition();
        GravitationalObject* newObj = createRandomObjectAt(mouse.x, mouse.y);
        addObjectList(newObj, objectList);

        if (1) 
        {
            printf("[CLICK] Neues Random Objekt erstellt bei %.2f, %.2f\n", mouse.x, mouse.y);
        }
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsKeyDown(KEY_LEFT_SHIFT))
    {
        firstPos = GetMousePosition();

        if (DEBUG_MODE) 
        {
            printf("[CLICK] Erste Position gesetzt bei %.2f, %.2f\n", firstPos.x, firstPos.y);
        }
    }
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && IsKeyDown(KEY_LEFT_SHIFT))  
    {
        Vector2 secondPos = GetMousePosition();
        Vector2 velocity = { (secondPos.x - firstPos.x), (secondPos.y - firstPos.y) };
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

    ObjectList* objectList = createObjectList();

    if (EXAMPELS) {
        GravitationalObject* earth = malloc(sizeof(*earth));
        *earth = (GravitationalObject){
            .name = "Earth",
            .mass = 50,
            .posX = 500,
            .posY = 600,
            .radius = 8,
            .color = BLUE,
            .forceX = 0,
            .forceY = 0,
            .velX = 0,
            .velY = -50
        };

        GravitationalObject* mars = malloc(sizeof(*mars));
        *mars = (GravitationalObject){
            .name = "Mars",
            .mass = 10,
            .posX = 600,
            .posY = 400,
            .radius = 7,
            .color = RED,
            .forceX = 0,
            .forceY = 0,
            .velX = -50,
            .velY = 0
        };

        GravitationalObject* sun = malloc(sizeof(*sun));
        *sun = (GravitationalObject){
            .name = "Sun",
            .mass = 250000,
            .posX = 1200./2.,
            .posY = 1000./2.,
            .radius = 25,
            .color = YELLOW,
            .forceX = 0,
            .forceY = 0,
            .velX = 0,
            .velY = 0
        };
        addObjectList(earth, objectList);
        addObjectList(mars, objectList);
        addObjectList(sun, objectList);
    }

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

        handleInput(objectList);

        while (t_temp >= t_tick) {
            CalculateGravitation(objectList);
            MoveParticles(objectList, t_tick);
            CalculateCollision(objectList);
            
            t_temp -= t_tick;
        }
        
        BeginDrawing();
            ClearBackground(BLACK);
            DrawParticles(objectList);
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


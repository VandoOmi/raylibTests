#include <raylib.h>
#include "particle.h"
#include "settings.h"

int main(){
    const int windowSizeX = 1960;
    const int windowSizeY = 1080;


    InitWindow(windowSizeX, windowSizeY, "Gravitations-Simulation");
    //SetWindowState(FLAG_FULLSCREEN_MODE);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 100.0f, 100.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;



    ObjectList* objectList = createObjectList();

    randomObjectsFor(1000, objectList, (Vector3){100, 100, 100});

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
            ComputeGravitationWithShader(objectList, t_tick);
            //MoveParticles(objectList, t_tick); // Already handled by shader
            CalculateCollision(objectList);
            t_temp -= t_tick;
        }
        
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                DrawGrid(200, 10.0f);
                DrawParticles(objectList);
            EndMode3D();
            //handleGUI(objectList);
        EndDrawing();
        printf("FPS: %.5i\n", GetFPS());

        if (DEBUG_MODE) {
            printf("---- Frame End --------------------------\n\n");
        }
       
    }

    //end
    CloseWindow();

    freeObjectList(objectList);

    return 0;
}
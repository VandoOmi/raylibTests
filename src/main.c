// Avoid Win32 macro/type conflicts (e.g., CloseWindow) by limiting windows.h
#define NOGDI
#define NOUSER
#include <raylib.h>
#include <GL/gl3w.h>
#include "particle.h"
#include "settings.h"

int main(){
    const int windowSizeX = 1960;
    const int windowSizeY = 1080;


    InitWindow(windowSizeX, windowSizeY, "Gravitations-Simulation");
    //SetWindowState(FLAG_FULLSCREEN_MODE);

    // Initialize OpenGL function loader (gl3w) AFTER the context is created
    if (gl3wInit()) {
        fprintf(stderr, "[ERROR] gl3wInit failed to initialize OpenGL loader.\n");
    }

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 100.0f, 100.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;



    ObjectList* objectList = createObjectList();
    InitParticleRender();

    randomObjectsFor(4000, objectList, (Vector3){1000, 1000, 1000});

    //loop
    float t_delta = 0;
    float t_tick = 1.0f / 90.0f; // physics tick
    float t_temp = 0;
    int frameCounter = 0;

    while(!WindowShouldClose()){
        
        t_delta = GetFrameTime();
        t_temp += t_delta;

        UpdateCamera(&camera, CAMERA_FREE);

        handleInput(objectList, &camera);
        // Runtime toggles
        if (IsKeyPressed(KEY_G)) SetUseGPU(!IsUseGPU());
        if (IsKeyPressed(KEY_C)) SetCullingEnabled(!IsCullingEnabled());

        // At most one physics substep per frame
        if (t_temp >= t_tick) {
            ComputeGravitationWithShader(objectList, t_tick);
            // Throttle collision checks (every 5 frames)
            if ((frameCounter % 5) == 0) {
                CalculateCollision(objectList);
            }
            t_temp -= t_tick;
        }
        
        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                DrawGrid(200, 10.0f);
                DrawParticles(objectList, &camera);
            EndMode3D();
            // HUD
            DrawText(TextFormat("Mode: %s  Culling: %s  Objects: %d FPS: %.5i", IsUseGPU()?"GPU":"CPU", IsCullingEnabled()?"On":"Off", objectList->size, GetFPS()), 10, 10, 20, RAYWHITE);
        EndDrawing();

        frameCounter++;
       
    }

    //end
    ShutdownParticleRender();
    CloseWindow();

    freeObjectList(objectList);

    return 0;
}
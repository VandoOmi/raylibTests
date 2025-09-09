#include <raylib.h> 

#define NUM_SPHERES 10

int main() {
    InitWindow(800, 600, "Mehrere Kugeln mit raylib");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Positionen der Kugeln
    Vector3 spherePositions[NUM_SPHERES];
    for (int i = 0; i < NUM_SPHERES; i++) {
        spherePositions[i] = (Vector3){ i * 2.0f, 1.0f, 0.0f }; // z.B. entlang der X-Achse
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        for (int i = 0; i < NUM_SPHERES; i++) {
            DrawSphere(spherePositions[i], 1.0f, RED);
        }

        DrawGrid(20, 1.0f);
        EndMode3D();

        DrawText("Kugeln aus Array", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

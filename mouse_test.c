#include "raylib.h"
#include <stdio.h>

int main(void)
{
    InitWindow(800, 600, "Maus-Test");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        Vector2 mouse = GetMousePosition();
        DrawCircleV(mouse, 10, RED);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            DrawText("KLICK!", 10, 10, 20, BLUE);
            printf("Mausklick erkannt bei x=%.1f y=%.1f\n", mouse.x, mouse.y);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

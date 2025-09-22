#include "InputHandler.h"

void handleInput(ObjectList* objectList, Camera3D* camera) {
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        Ray mouseRay = GetMouseRay(GetMousePosition(), *camera);
        Vector3 pos = Vector3Add(camera->position, Vector3Scale(mouseRay.direction, 100.0f));
        GravitationalObject* newObj = createRandomParticleAt(&pos);
        addObjectList(newObj, objectList);
    }
    // Additional input handling for custom object creation can be added here
}
#include "Draw.h"

const int PARTICLERADIUS = 1; // in km

// Get color for a given element type
Color getColor(enum element element) {
    switch (element) {
        case hydrogen: return RAYWHITE;
        case helium:   return RED;
        case oxygen:   return BLUE;
        case carbon:   return GRAY;
        case neon:     return PINK;
        case iron:     return LIGHTGRAY;
    }
    return WHITE;
}

// Cached sphere model for faster rendering
static Model gSphereModel = {0};
static bool gSphereReady = false;

void InitParticleRender(void) {
    if (gSphereReady) return;
    Mesh sphere = GenMeshSphere((float)PARTICLERADIUS, 8, 8);
    gSphereModel = LoadModelFromMesh(sphere);
    gSphereReady = true;
}

void ShutdownParticleRender(void) {
    if (!gSphereReady) return;
    UnloadModel(gSphereModel);
    gSphereModel = (Model){0};
    gSphereReady = false;
}

// Draw a single particle using the cached sphere model
static inline void drawParticle(GravitationalObject *obj) {
    if (!gSphereReady) InitParticleRender();
    Color col = getColor(obj->element);
    Vector3 pos = obj->position;
    DrawModel(gSphereModel, pos, 1.0f, col);
    if (DEBUG_MODE) {
        printf("[DRAW] %s: pos=(%.2f, %.2f, %.2f)\n", obj->name, pos.x, pos.y, pos.z);
    }
}

// Frustum culling: quick sphere-in-frustum test using camera FOV and orientation
static inline int SphereInView(const Camera3D* cam, Vector3 center, float radius) {
    // Camera basis vectors
    Vector3 forward = Vector3Normalize(Vector3Subtract(cam->target, cam->position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, cam->up));
    Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

    // Vector from camera to sphere center
    Vector3 v = Vector3Subtract(center, cam->position);

    // Depth along forward axis
    float z = Vector3DotProduct(v, forward);
    if (z < -radius) return 0; // behind camera

    // Near/far planes (approximate to typical values used by raylib)
    const float nearPlane = 0.01f;
    const float farPlane = 10000.0f;
    // Add small margin to reduce popping near near-plane
    float margin = 2.0f;
    if (z + radius < nearPlane - margin) return 0;
    if (z - radius > farPlane) return 0;

    // Field of view
    float aspect = (float)GetScreenWidth() / (float)GetScreenHeight();
    float vHalf = tanf((cam->fovy * DEG2RAD) * 0.5f);
    float hHalf = vHalf * aspect;

    // Project v onto camera right and up axes to get horizontal/vertical distances
    float x = Vector3DotProduct(v, right);
    float y = Vector3DotProduct(v, up);

    // Frustum side planes: |x| <= z * hHalf (+radius), |y| <= z * vHalf (+radius)
    // Use max(z, nearPlane) to avoid negative z cases and add small margin
    float zClamp = (z < nearPlane) ? nearPlane : z;
    if (fabsf(x) > zClamp * hHalf + radius + margin) return 0;
    if (fabsf(y) > zClamp * vHalf + radius + margin) return 0;

    return 1;
}

// Draw all particles in the object list (only those in camera view)
void DrawParticles(ObjectList* oList, const Camera3D* camera, bool gCullingEnabled) {
    for (int i = 0; i < oList->size; i++) {
        GravitationalObject* obj = oList->gObjs[i];
        if (!gCullingEnabled || SphereInView(camera, obj->position, (float)PARTICLERADIUS)) {
            drawParticle(obj);
        }
    }
}
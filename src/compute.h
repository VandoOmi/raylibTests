#ifndef COMPUTE_H
#define COMPUTE_H

#define NOGDI
#define NOUSER
#include <raylib.h>
#include <GL/gl3w.h>
#include <stdio.h>
#include <stdlib.h>


// std430 layout alignment:
// - vec3 has 16-byte base alignment, so add padding after 3 floats
// - structure aligned to 16 bytes; total size becomes 48 bytes
typedef struct GPUObject {
    float position[3]; float _padPos;   // 16 bytes
    float velocity[3]; float _padVel;   // 16 bytes
    float mass;        float _padTail[3]; // 16 bytes
} GPUObject;

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert(sizeof(GPUObject) == 48, "GPUObject must be 48 bytes (std430 vec3 alignment)");
#endif

// Returns non-zero if compute path is usable on this machine (GL 4.3+ and context ready)
int computeAvailable(void);

GLuint createGravityComputeShader();

// Returns 1 on success, 0 on failure (caller can fall back to CPU path)
int computeGravity(GPUObject* objects, int numObjects, float deltatime);

#endif
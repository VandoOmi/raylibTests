#ifndef COMPUTE_H
#define COMPUTE_H

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <GL/gl3w.h>

typedef struct GPUObject GPUObject;

GLuint createGravityComputeShader();

void computeGravity(GPUObject* objects, int numObjects, float deltatime);

#endif
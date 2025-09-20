#ifndef COMPUTE_H
#define COMPUTE_H

#define NOGDI
#define NOUSER
#include <raylib.h>
#include <GL/gl3w.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct GPUObject {
	float position[3];
	float velocity[3];
	float mass;
} GPUObject;

GLuint createGravityComputeShader();

void computeGravity(GPUObject* objects, int numObjects, float deltatime);

#endif
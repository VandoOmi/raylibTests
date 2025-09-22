

#include "GridSystemGravity_CS.h"
#include <stdio.h>
#include "compute.h" // for GPUObject
#include <raylib.h>   // for Vector3 if needed

GLuint createGravityComputeShader() {
    char* computeShaderSrc = LoadFileText("shader/GridGravitation.comp");
    if (!computeShaderSrc) {
        printf("[createGravityComputeShader] ERROR: Could not load shader file at 'shader/gravitation.comp'.\n");
        return 0;
    }
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, (const GLchar* const*)&computeShaderSrc, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("[createGravityComputeShader] Compile error: %s\n", infoLog);
        glDeleteShader(shader);
        UnloadFileText(computeShaderSrc);
        return 0;
    }
    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    glDeleteShader(shader);
    UnloadFileText(computeShaderSrc);
    GLint linkOK = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linkOK);
    if (!linkOK) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("[createGravityComputeShader] Link error: %s\n", infoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

int computeGridGravity(GPUObject* objects, int numObjects, GPUGridCell* cells, int numCells, unsigned int* objIndices, int numObjIndices, Vector3 gridSize, float cellSize, float deltatime, float G) {
    static GLuint shaderProgram = 0;
    static GLuint ssboObjects = 0;
    static GLuint ssboCells = 0;
    static GLuint ssboObjIndices = 0;
    static int prevNumObjects = 0, prevNumCells = 0, prevNumObjIndices = 0;

    if (shaderProgram == 0) {
        shaderProgram = createGravityComputeShader();
        if (shaderProgram == 0) return 0;
    }

    // Allocate or reallocate SSBOs if sizes change
    if (ssboObjects == 0 || prevNumObjects != numObjects) {
        if (ssboObjects != 0) { glDeleteBuffers(1, &ssboObjects); ssboObjects = 0; }
        glGenBuffers(1, &ssboObjects);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboObjects);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUObject) * numObjects, objects, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        prevNumObjects = numObjects;
    } else {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboObjects);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GPUObject) * numObjects, objects);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    if (ssboCells == 0 || prevNumCells != numCells) {
        if (ssboCells != 0) { glDeleteBuffers(1, &ssboCells); ssboCells = 0; }
        glGenBuffers(1, &ssboCells);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCells);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUGridCell) * numCells, cells, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        prevNumCells = numCells;
    } else {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCells);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GPUGridCell) * numCells, cells);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    if (ssboObjIndices == 0 || prevNumObjIndices != numObjIndices) {
        if (ssboObjIndices != 0) { glDeleteBuffers(1, &ssboObjIndices); ssboObjIndices = 0; }
        glGenBuffers(1, &ssboObjIndices);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboObjIndices);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * numObjIndices, objIndices, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        prevNumObjIndices = numObjIndices;
    } else {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboObjIndices);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int) * numObjIndices, objIndices);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    glUseProgram(shaderProgram);
    glUniform1f(glGetUniformLocation(shaderProgram, "deltaTime"), deltatime);
    glUniform1f(glGetUniformLocation(shaderProgram, "G"), G);
    glUniform3ui(glGetUniformLocation(shaderProgram, "gridSize"), (unsigned int)gridSize.x, (unsigned int)gridSize.y, (unsigned int)gridSize.z);
    glUniform1f(glGetUniformLocation(shaderProgram, "cellSize"), cellSize);

    // Bind buffers to match compute shader bindings
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboObjects);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboCells);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboObjIndices);

    // Dispatch compute shader
    glDispatchCompute((numObjects + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // Read back results from GPU to CPU (from objects buffer)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboObjects);
    GPUObject* ptr = (GPUObject*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    if (ptr) {
        memcpy(objects, ptr, sizeof(GPUObject) * numObjects);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return 1;
}


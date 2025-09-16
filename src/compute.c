#include "compute.h"

typedef struct GPUObject {
    float position[3];
    float velocity[3];
    float mass;
} GPUObject;

GLuint createGravityComputeShader() {
    const char* computeShaderSrc = LoadFileText("shader/gravitation.comp");

    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &computeShaderSrc, NULL);
    glCompileShader(shader);

    // Check compilation
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR: %s\n", infoLog);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    glDeleteShader(shader); // shader not needed after linking

    return program;
}

void computeGravity(GPUObject* objects, int numObjects, float deltatime) {
    static GLuint shaderProgram = 0;
    static GLuint ssbo = 0;

    if (shaderProgram == 0) shaderProgram = createGravityComputeShader();

    if (ssbo == 0) {
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUObject) * numObjects, objects, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    } else {
        // Update buffer with new data if needed
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GPUObject) * numObjects, objects);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    glUseProgram(shaderProgram);
    glUniform1f(glGetUniformLocation(shaderProgram, "dt"), deltatime);
    glUniform1f(glGetUniformLocation(shaderProgram, "G"), 6.67430e-11f);
    glUniform1i(glGetUniformLocation(shaderProgram, "numObjects"), numObjects);

    glDispatchCompute((numObjects + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Read back results
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    GPUObject* ptr = (GPUObject*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    if (ptr) {
        for (int i = 0; i < numObjects; i++) objects[i] = ptr[i];
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

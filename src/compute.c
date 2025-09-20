
#include "compute.h"
#include "settings.h"


GLuint createGravityComputeShader() {
    const GLubyte* glVersion = glGetString(GL_VERSION);
    if (DEBUG_MODE && glVersion) printf("[createGravityComputeShader] OpenGL Version: %s\n", glVersion);
    if (DEBUG_MODE) printf("[createGravityComputeShader] Loading shader file...\n");
    const char* computeShaderSrc = LoadFileText("shader/gravitation.comp");
    if (DEBUG_MODE) printf("[createGravityComputeShader] Shader file loaded.\n");
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    if (DEBUG_MODE) printf("[createGravityComputeShader] Shader object created: %u\n", shader);
    if (shader == 0) {
        printf("[createGravityComputeShader] ERROR: glCreateShader(GL_COMPUTE_SHADER) returned 0!\n");
    }
    glShaderSource(shader, 1, &computeShaderSrc, NULL);
    glCompileShader(shader);
    if (DEBUG_MODE) printf("[createGravityComputeShader] Shader compiled. Checking for errors...\n");
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("[createGravityComputeShader] ERROR: %s\n", infoLog);
    } else {
        if (DEBUG_MODE) printf("[createGravityComputeShader] Shader compiled successfully.\n");
    }
    GLuint program = glCreateProgram();
    if (DEBUG_MODE) printf("[createGravityComputeShader] Program object created: %u\n", program);
    glAttachShader(program, shader);
    glLinkProgram(program);
    if (DEBUG_MODE) printf("[createGravityComputeShader] Program linked. Deleting shader object...\n");
    glDeleteShader(shader);
    if (DEBUG_MODE) printf("[createGravityComputeShader] Shader object deleted.\n", program);
    return program;
}

void computeGravity(GPUObject* objects, int numObjects, float deltatime) {
    if (DEBUG_MODE) printf("[computeGravity] Called with %d objects, deltaTime=%.6f\n", numObjects, deltatime);
    static GLuint shaderProgram = 0; // Handle to the compute shader program
    static GLuint ssbo = 0;          // Handle to the shader storage buffer object
    static int prevNumObjects = 0;   // Track previous number of objects for buffer reallocation

    // Create shader program if not already created
    if (shaderProgram == 0) {
        if (DEBUG_MODE) printf("[computeGravity] Creating compute shader program...\n");
        shaderProgram = createGravityComputeShader();
        if (DEBUG_MODE) printf("[computeGravity] Shader program created: %u\n", shaderProgram);
    }
    if (DEBUG_MODE) printf("[computeGravity] After shader program creation.\n");
    if (DEBUG_MODE) printf("[computeGravity] Before SSBO allocation/update.\n");

    // Allocate or reallocate SSBO if number of objects changes
    if (ssbo == 0 || prevNumObjects != numObjects) {
        if (ssbo != 0) {
            if (DEBUG_MODE) printf("[computeGravity] Deleting old SSBO...\n");
            glDeleteBuffers(1, &ssbo);
            ssbo = 0;
        }
        if (DEBUG_MODE) printf("[computeGravity] Generating new SSBO for %d objects...\n", numObjects);
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUObject) * numObjects, objects, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        prevNumObjects = numObjects;
        if (DEBUG_MODE) printf("[computeGravity] After SSBO allocation.\n");
    } else {
        // Update buffer with new data if number of objects is unchanged
        if (DEBUG_MODE) printf("[computeGravity] Updating SSBO with %d objects...\n", numObjects);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GPUObject) * numObjects, objects);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        if (DEBUG_MODE) printf("[computeGravity] After SSBO update.\n");
    }

    // Set shader uniforms for simulation step
    if (DEBUG_MODE) printf("[computeGravity] Setting uniforms and dispatching compute shader...\n");
    glUseProgram(shaderProgram);
    if (DEBUG_MODE) printf("[computeGravity] After glUseProgram.\n");
    glUniform1f(glGetUniformLocation(shaderProgram, "deltaTime"), deltatime);
    if (DEBUG_MODE) printf("[computeGravity] After setting deltaTime uniform.\n");
    glUniform1f(glGetUniformLocation(shaderProgram, "G"), 6.67430e-11f);
    if (DEBUG_MODE) printf("[computeGravity] After setting G uniform.\n");
    glUniform1i(glGetUniformLocation(shaderProgram, "numObjects"), numObjects);
    if (DEBUG_MODE) printf("[computeGravity] After setting numObjects uniform.\n");

    // Dispatch compute shader with enough workgroups for all objects
    glDispatchCompute((numObjects + 255) / 256, 1, 1);
    if (DEBUG_MODE) printf("[computeGravity] After glDispatchCompute.\n");
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    if (DEBUG_MODE) printf("[computeGravity] After glMemoryBarrier.\n");
    if (DEBUG_MODE) printf("[computeGravity] Compute shader dispatched.\n");

    // Read back results from GPU to CPU
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    if (DEBUG_MODE) printf("[computeGravity] After glBindBuffer for readback.\n");
    GPUObject* ptr = (GPUObject*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    if (DEBUG_MODE) printf("[computeGravity] After glMapBuffer.\n");
    if (ptr) {
        if (DEBUG_MODE) printf("[computeGravity] ptr is valid, copying data.\n");
        for (int i = 0; i < numObjects; i++) objects[i] = ptr[i];
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        if (DEBUG_MODE) printf("[computeGravity] After glUnmapBuffer.\n");
    } else {
        if (DEBUG_MODE) printf("[computeGravity] ERROR: glMapBuffer returned NULL!\n");
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    if (DEBUG_MODE) printf("[computeGravity] End of function.\n");
}

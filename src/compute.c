
#include "compute.h"
#include "settings.h"


int computeAvailable(void) {
    // Ensure function pointers exist and version >= 4.3 for compute shaders
    const GLubyte* versionStr = glGetString(GL_VERSION);
    if (!versionStr) return 0;
    int major = 0, minor = 0;
    if (sscanf((const char*)versionStr, "%d.%d", &major, &minor) < 2) return 0;
    int available = (major > 4) || (major == 4 && minor >= 3);
    return available;
}

GLuint createGravityComputeShader() {
    const GLubyte* glVersion = glGetString(GL_VERSION);
    char* computeShaderSrc = LoadFileText("shader/gravitation.comp");
    if (!computeShaderSrc) {
        printf("[createGravityComputeShader] ERROR: Could not load shader file at 'shader/gravitation.comp'.\n");
        return 0;
    }
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    if (shader == 0) {
        printf("[createGravityComputeShader] ERROR: glCreateShader(GL_COMPUTE_SHADER) returned 0!\n");
        UnloadFileText(computeShaderSrc);
        return 0;
    }
    const GLchar* const srcs[] = { (const GLchar*)computeShaderSrc };
    glShaderSource(shader, 1, srcs, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        glDeleteShader(shader);
        UnloadFileText(computeShaderSrc);
        return 0;
    } else {
    }
    GLuint program = glCreateProgram();
    if (program == 0) {
        glDeleteShader(shader);
        UnloadFileText(computeShaderSrc);
        return 0;
    }
    glAttachShader(program, shader);
    glLinkProgram(program);
    glDeleteShader(shader);
    UnloadFileText(computeShaderSrc);

    // Check link success
    GLint linkOK = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linkOK);
    if (!linkOK) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

int computeGravity(GPUObject* objects, int numObjects, float deltatime) {
    static GLuint shaderProgram = 0; // Handle to the compute shader program
    static GLuint ssboIn = 0;        // Input buffer (binding = 0)
    static GLuint ssboOut = 0;       // Output buffer (binding = 1)
    static int prevNumObjects = 0;   // Track previous number of objects for buffer reallocation

    // Create shader program if not already created
    if (shaderProgram == 0) {
        if (!computeAvailable()) {
            return 0;
        }
        shaderProgram = createGravityComputeShader();
        if (shaderProgram == 0) {
            return 0;
        }
    }

    // Allocate or reallocate SSBOs if number of objects changes
    if (ssboIn == 0 || ssboOut == 0 || prevNumObjects != numObjects) {
        if (ssboIn != 0) { glDeleteBuffers(1, &ssboIn); ssboIn = 0; }
        if (ssboOut != 0) { glDeleteBuffers(1, &ssboOut); ssboOut = 0; }
        glGenBuffers(1, &ssboIn);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIn);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUObject) * numObjects, objects, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glGenBuffers(1, &ssboOut);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOut);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUObject) * numObjects, NULL, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        prevNumObjects = numObjects;
    } else {
        // Update input buffer with latest CPU data
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIn);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GPUObject) * numObjects, objects);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    // Set shader uniforms for simulation step
    glUseProgram(shaderProgram);
    glUniform1f(glGetUniformLocation(shaderProgram, "deltaTime"), deltatime);
    glUniform1f(glGetUniformLocation(shaderProgram, "G"), 6.67430e-1f);
    glUniform1i(glGetUniformLocation(shaderProgram, "numObjects"), numObjects);
    glUniform1f(glGetUniformLocation(shaderProgram, "softening"), 1e-6f);
    glUniform1f(glGetUniformLocation(shaderProgram, "maxSpeed"), 1000.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "maxPos"), 100000.0f);

    // Bind buffers to match compute shader bindings
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboIn);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboOut);

    // Dispatch compute shader with enough workgroups for all objects
    glDispatchCompute((numObjects + 255) / 256, 1, 1);
    // Ensure writes to SSBO are visible before mapping
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // Read back results from GPU to CPU (from output buffer)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOut);
    GPUObject* ptr = (GPUObject*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    if (ptr) {
        for (int i = 0; i < numObjects; i++) {
            GPUObject o = ptr[i];
            // Basic NaN guard on CPU side as a last resort
            if (!(o.position[0] == o.position[0])) { o.position[0] = 0.0f; }
            if (!(o.position[1] == o.position[1])) { o.position[1] = 0.0f; }
            if (!(o.position[2] == o.position[2])) { o.position[2] = 0.0f; }
            if (!(o.velocity[0] == o.velocity[0])) { o.velocity[0] = 0.0f; }
            if (!(o.velocity[1] == o.velocity[1])) { o.velocity[1] = 0.0f; }
            if (!(o.velocity[2] == o.velocity[2])) { o.velocity[2] = 0.0f; }
            objects[i] = o;
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    } else {
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return 1;
}

# AI contributor quickstart for this repo

This repository is a C/raylib project simulating particle gravitation with an optional GPU compute path. It builds with CMake (Ninja or MinGW) and runs a desktop GLFW window.

## Big picture
- Entry point: `src/main.c` sets up a `Camera3D`, creates an `ObjectList`, and runs the main loop.
- Simulation data: `GravitationalObject` and `ObjectList` are defined in `src/particle.h` and implemented in `src/particle.c`.
- GPU compute path: `src/compute.c` + `src/compute.h` implement OpenGL compute shader execution over a struct array (`GPUObject`) via SSBOs and read the results back to CPU memory. Shader source is `shader/gravitation.comp`.
- Flow each frame (simplified):
  - Input/camera → `handleInput`
  - Physics tick(s) → `ComputeGravitationWithShader` (GPU) or `CalculateGravitation` (CPU legacy)
  - Collisions → `CalculateCollision`
  - Draw → `DrawParticles` within `BeginMode3D/EndMode3D`

Why it’s structured this way: CPU code provides a reference; GPU path accelerates the O(N^2) step while preserving the same data model. The shader and `GPUObject` memory layout must match.

## Build & run
- Standard build (top-level):
  - Configure: `cmake -S . -B build -G Ninja`
  - Build: `cmake --build build`
  - Run from build dir so relative assets work: `./graviton`
- Toolchain: repo is configured for MinGW/Ninja on Windows (GLFW via raylib). Other platforms should work with equivalent CMake toolchains.

## OpenGL requirements (critical)
- Current default is `GRAPHICS_API_OPENGL_33` (no compute). Compute shaders require OpenGL 4.3+.
- To enable compute:
  - Rebuild bundled raylib in `external/raylib` with MinGW: `cmake -B build -G "MinGW Makefiles" -DPLATFORM=Desktop -DGRAPHICS=OpenGL -DGRAPHICS_API_OPENGL_43=ON && cmake --build build --config Release` (delete its `build/` first if switching generators).
  - Also define `GRAPHICS_API_OPENGL_43` for this app (e.g., add `add_definitions(-DGRAPHICS_API_OPENGL_43)` in root `CMakeLists.txt`).
- All OpenGL calls must happen after `InitWindow()` creates the context. Don’t call GL in global initializers.

## Assets & paths
- The compute shader is loaded with a relative path: `LoadFileText("shader/gravitation.comp")`.
- Run from the project root or copy `shader/gravitation.comp` into `build/shader/` so the file exists at runtime.

## Conventions & patterns
- Headers: include `raylib.h` before OpenGL loader headers; on Windows, `compute.h` defines `#define NOGDI` and `#define NOUSER` to avoid Win32 macro conflicts (e.g., `Rectangle`).
- Debugging: `settings.h` defines `DEBUG_MODE`. Most verbose logs in `compute.c` are wrapped with `if (DEBUG_MODE)` for easy on/off.
- Data layout contract: `GPUObject` in `compute.h` mirrors the GLSL `struct Object` in `shader/gravitation.comp`:
  - C: `float position[3]; float velocity[3]; float mass;`
  - GLSL: `vec3 position; vec3 velocity; float mass;`
  Keep field order, sizes, and std430 alignment in sync.
- SSBO binding: SSBO is bound at `binding = 0` and updated every frame; dispatch uses `local_size_x = 256` and groups `(numObjects + 255)/256`.

## Common pitfalls (seen in this repo)
- Shader missing at runtime → ensure `shader/gravitation.comp` is reachable from working directory.
- Crashes on GL calls → context not created yet or OpenGL 4.3 not enabled; only call after `InitWindow()` and rebuild raylib/app for 4.3.
- Type sync errors → don’t forward-declare structs you dereference; keep full `GravitationalObject` in `particle.h`.
- Duplicate type defs → define `ObjectList` only once (in header), not again in `.c`.

## Where to look
- Main loop: `src/main.c`
- CPU physics, collisions, draw: `src/particle.c` + `src/particle.h`
- GPU compute pipeline: `src/compute.c` + `src/compute.h`
- Shader: `shader/gravitation.comp`
- Vendor libs: `external/raylib` (GLFW inside), `external/gl3w`

## When adding features
- If adding new fields to `GravitationalObject` that affect simulation, update `GPUObject`, the GLSL `Object` struct, and the host/device copy logic in `ComputeGravitationWithShader` and `computeGravity`.
- If adding assets, update runtime paths or add CMake copy steps so assets are in `build/` at run.
- Prefer wrapping verbose logs in `DEBUG_MODE` and keep GL calls after context creation.

Feedback needed: If any of these steps don’t match your local setup (e.g., different generator, copy rules, or you move files), tell us so we can update this guide.
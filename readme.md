# Graviton: 3D Gravitational Simulation

**Graviton** is a real-time 3D gravitational simulation written in C, using [raylib](https://www.raylib.com/) for graphics, [raygui](https://github.com/raysan5/raygui) for GUI, and OpenGL compute shaders for physics calculations. The project visualizes thousands of particles interacting under Newtonian gravity.

## Features

- Real-time 3D visualization of gravitational interactions
- GPU-accelerated physics using OpenGL compute shaders
- Interactive camera controls
- Highly performant, simulating thousands of particles

## Screenshots

*(Add your screenshots here!)*

## Getting Started

### Prerequisites

- C compiler (GCC, Clang, or MSVC)
- [CMake](https://cmake.org/) 3.10 or higher
- [raylib](https://www.raylib.com/) (included as a submodule)
- [raygui](https://github.com/raysan5/raygui) (included as a submodule)
- OpenGL 4.3+ capable GPU

### Build Instructions

1. **Clone the repository (with submodules):**
   ```sh
   git clone --recurse-submodules <repo-url>
   ```
   If you already cloned without submodules, run:
   ```sh
   git submodule update --init --recursive
   ```

2. **Build with CMake:**
   ```sh
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. **Run the simulation:**
   - On Windows: `./graviton.exe`
   - On Linux/macOS: `./graviton`

### Project Structure

- `src/` — Main source code (simulation, rendering, particle logic)
- `shader/` — Compute shader for physics calculations
- `external/raylib/` — raylib library (as submodule)
- `external/raygui/` — raygui GUI library (as submodule)
- `external/gl3w/` — OpenGL loader
- `CMakeLists.txt` — Build configuration

### Customization

- Change the number of particles or simulation parameters in `src/main.c` and `src/particle.h`.
- Modify the compute shader in `shader/gravitation.comp` for custom physics.
- Extend the GUI using raygui in `external/raygui/` and enable GUI code in `src/main.c`.

## Credits

- [raylib](https://www.raylib.com/) — easy-to-use graphics library
- [raygui](https://github.com/raysan5/raygui) — immediate-mode GUI for raylib
- [gl3w](https://github.com/skaslev/gl3w) — OpenGL core profile loader

## License

This project is open source. See `LICENSE` for details.

cmake_minimum_required(VERSION 3.10)

project(Graviton C)   # Projektname + Sprache

set(CMAKE_C_STANDARD 99)

# Force raylib to build with OpenGL 4.3 (compute shaders)
set(GRAPHICS OpenGL CACHE STRING "Raylib graphics backend")
set(GRAPHICS_API_OPENGL_43 ON CACHE BOOL "Enable OpenGL 4.3 in raylib")

# Raylib als Submodule hinzufügen (after setting cache options)
# Force raylib to build with OpenGL 4.3 (compute shaders)
set(PLATFORM Desktop CACHE STRING "")
set(GRAPHICS OpenGL CACHE STRING "Raylib graphics backend")
set(GRAPHICS_API_OPENGL_43 ON CACHE BOOL "Enable OpenGL 4.3 in raylib")

# Raylib als Submodule hinzufügen (after setting cache options)
add_subdirectory(external/raylib)

add_library(gl3w external/gl3w/src/gl3w.c)
target_include_directories(gl3w PUBLIC external/gl3w/include)

# Executable anlegen
add_executable(graviton
    src/main.c 
    src/particle.c
    src/compute.c
)

# Mit Raylib linken
target_link_libraries(graviton raylib gl3w)

# Raygui-Header einbinden
target_include_directories(graviton PRIVATE ${CMAKE_SOURCE_DIR}/external/raygui/src)

# Windows-spezifische Libs
if (WIN32)
    target_link_libraries(graviton winmm gdi32 opengl32)
endif()

# Linux-spezifische Libs
if (UNIX AND NOT APPLE)
    target_link_libraries(graviton m pthread dl GL rt X11)
endif()

# Define OpenGL 4.3 API for compute shaders in our app
add_definitions(-DGRAPHICS_API_OPENGL_43)

# Copy shader assets next to the executable for runtime
add_custom_command(TARGET graviton POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:graviton>/shader
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/shader/gravitation.comp
            $<TARGET_FILE_DIR:graviton>/shader/gravitation.comp
)

if(APPLE)
    target_link_libraries(graviton
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework CoreVideo"
        "-framework CoreAudio"
        "-framework AudioToolbox"
    )
endif()


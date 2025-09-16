cmake_minimum_required(VERSION 3.10)

project(Graviton C)   # Projektname + Sprache

set(CMAKE_C_STANDARD 99)

# Raylib als Submodule hinzufügen
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


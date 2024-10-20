cmake_minimum_required(VERSION 3.29)
project(something C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c)

# Create the executable
add_executable(something main.c
        queue.c)

# Link the executable with GLFW and OpenGL
target_link_libraries(something -lOpenGL32 -lfreeGLUT -static -static-libgcc)

cmake_minimum_required(VERSION 3.19)

project(
    glow
    VERSION 0.1
    LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(DEFAULT_BUILD_TYPE "Release")

# ----------------------------------------------------------------------------------------

set(FILE_SOURCES
    src/camera.c
    src/maths.c
    src/opengl.c
    src/shader.c
    src/texture.c
    src/main.c)

set(FILE_HEADERS
    src/camera.h
    src/console.h
    src/maths.h
    src/opengl.h
    src/shader.h
    src/texture.h
    src/prelude.h)

# ----------------------------------------------------------------------------------------

add_subdirectory(ext/)

add_executable(
    ${PROJECT_NAME}
    ${FILE_SOURCES}
    ${FILE_HEADERS})

# ----------------------------------------------------------------------------------------

target_compile_definitions(${PROJECT_NAME} PRIVATE GLOW_SHADERS_="${PROJECT_SOURCE_DIR}/src/shaders/")
target_compile_definitions(${PROJECT_NAME} PRIVATE GLOW_TEXTURES_="${PROJECT_SOURCE_DIR}/res/textures/")

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(${PROJECT_NAME} glfw glad stb)

# ----------------------------------------------------------------------------------------

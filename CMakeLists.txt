cmake_minimum_required(VERSION 3.19)

project(glow LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(DEFAULT_BUILD_TYPE "RelWithDebInfo")

# ----------------------------------------------------------------------------------------

set(FILE_SOURCES
    src/camera.c
    src/color.c
    src/dynarray.c
    src/file.c
    src/fullscreen_quad.c
    src/imgui_facade.cpp
    src/maths.c
    src/mesh.c
    src/model_assimp.inl
    src/model.c
    src/opengl.c
    src/options.c
    src/shader.c
    src/texture.c
    src/window.inl
    src/main.inl
    src/main.c)

set(FILE_HEADERS
    src/camera.h
    src/color.h
    src/console.h
    src/dynarray.h
    src/file.h
    src/fullscreen_quad.h
    src/imgui_facade.h
    src/maths_types.h
    src/maths.h
    src/mesh.h
    src/model.h
    src/opengl.h
    src/options.inc
    src/options.h
    src/shader.h
    src/texture.h
    src/vertices.h
    src/window.h
    src/prelude.h)

# ----------------------------------------------------------------------------------------

add_subdirectory(ext/)

add_executable(
    ${PROJECT_NAME}
    ${FILE_SOURCES}
    ${FILE_HEADERS})

# ----------------------------------------------------------------------------------------

target_compile_definitions(${PROJECT_NAME}
    PRIVATE GLOW_MODELS_="${PROJECT_SOURCE_DIR}/res/models/"
    PRIVATE GLOW_SHADERS_="${PROJECT_SOURCE_DIR}/res/shaders/"
    PRIVATE GLOW_TEXTURES_="${PROJECT_SOURCE_DIR}/res/textures/"

    PUBLIC  GLFW_INCLUDE_NONE

    PUBLIC  _CRT_SECURE_NO_WARNINGS)

target_link_libraries(${PROJECT_NAME} glfw glad stb assimp imgui)

# ----------------------------------------------------------------------------------------

option(GLOW_WARNINGS "GLOW_WARNINGS" OFF)

if(GLOW_WARNINGS)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /W4) # /WX
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic) # -Werror
    endif()
endif()

# ----------------------------------------------------------------------------------------

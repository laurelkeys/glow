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
    src/main.c)

set(FILE_HEADERS
    src/maths.h
    src/prelude.h)

# ----------------------------------------------------------------------------------------

# Note: keep `add_subdirectory` after `include_directories`.

include_directories(
    ext/glfw/include/
    ext/glad/include/)

add_subdirectory(ext/)

add_executable(
    ${PROJECT_NAME}
    ${FILE_SOURCES}
    ${FILE_HEADERS})

# ----------------------------------------------------------------------------------------

target_link_libraries(${PROJECT_NAME} glfw glad)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

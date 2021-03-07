cmake_minimum_required(VERSION 3.19)

project(
    glow
    VERSION 0.1
    LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# if(MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
# else()
#     target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
# endif()

set(DEFAULT_BUILD_TYPE "Release")

# ----------------------------------------------------------------------------------------

set(FILE_SOURCES
    src/main.c)

set(FILE_HEADERS
    src/maths.h
    src/prelude.h)

# ----------------------------------------------------------------------------------------

include_directories(
    ext/glfw/include/
    ext/glad/include/)

add_subdirectory(ext/)  # Note: keep this after `include_directories`

# ----------------------------------------------------------------------------------------

add_executable(
    ${PROJECT_NAME}
    ${FILE_SOURCES}
    ${FILE_HEADERS})

# ----------------------------------------------------------------------------------------

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

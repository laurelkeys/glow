cmake_minimum_required(VERSION 3.19)

# glfw
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
add_subdirectory(glfw)

# glad
add_library(glad STATIC glad/src/glad.c)
if(NOT WIN32)
    target_link_libraries(glad PUBLIC dl)
endif()

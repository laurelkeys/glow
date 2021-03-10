cmake_minimum_required(VERSION 3.19)

# ----------------------------------------------------------------------------------------

option(GLFW_BUILD_DOCS ON)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
add_subdirectory(glfw)

# ----------------------------------------------------------------------------------------

add_library(glad STATIC glad/src/glad.c)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})
target_include_directories(glad PUBLIC glad/include/)

# ----------------------------------------------------------------------------------------

# add_library(stb INTERFACE)
# target_include_directories(stb INTERFACE stb/)
add_library(stb STATIC stb/stb_image.c)
target_include_directories(stb PUBLIC stb/)

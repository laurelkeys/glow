cmake_minimum_required(VERSION 3.19)

# ----------------------------------------------------------------------------------------

option(GLFW_INSTALL OFF)
option(GLFW_BUILD_DOCS ON)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_EXAMPLES OFF)

add_subdirectory(glfw)

# ----------------------------------------------------------------------------------------

add_library(glad STATIC)
target_sources(glad PRIVATE glad/src/glad.c)
target_include_directories(glad SYSTEM PUBLIC glad/include/)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

# ----------------------------------------------------------------------------------------

# add_library(stb INTERFACE)
# target_include_directories(stb SYSTEM INTERFACE stb/)

add_library(stb STATIC)
target_sources(stb PRIVATE stb/stb_impl.c)
target_include_directories(stb SYSTEM PUBLIC stb/)

# ----------------------------------------------------------------------------------------

option(ASSIMP_INSTALL OFF)
option(ASSIMP_NO_EXPORT ON)
option(ASSIMP_BUILD_ZLIB OFF)
option(ASSIMP_BUILD_TESTS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
option(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)

set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)
set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)
# set(ASSIMP_BUILD_FBX_IMPORTER TRUE)

add_subdirectory(assimp)

# ----------------------------------------------------------------------------------------

set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp)

set(IMGUI_BACKEND_SOURCES
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp)

add_library(imgui STATIC)
target_sources(imgui PRIVATE ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})
target_include_directories(imgui SYSTEM PUBLIC imgui/ imgui/backends/)
target_link_libraries(imgui PUBLIC glfw glad)

# ----------------------------------------------------------------------------------------

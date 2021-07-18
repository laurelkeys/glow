cmake_minimum_required(VERSION 3.19)

# ----------------------------------------------------------------------------------------

option(GLFW_BUILD_DOCS ON)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_EXAMPLES OFF)

add_subdirectory(glfw)

# ----------------------------------------------------------------------------------------

add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include/)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

# ----------------------------------------------------------------------------------------

# add_library(stb INTERFACE)
# target_include_directories(stb INTERFACE stb/)

add_library(stb STATIC stb/stb_impl.c)
target_include_directories(stb PUBLIC stb/)

# ----------------------------------------------------------------------------------------

option(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_BLEND_IMPORTER TRUE)
set(ASSIMP_BUILD_COLLADA_IMPORTER TRUE)
set(ASSIMP_BUILD_FBX_IMPORTER TRUE)
set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)
set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)
set(ASSIMP_BUILD_PLY_IMPORTER TRUE)
set(ASSIMP_BUILD_STL_IMPORTER TRUE)

option(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
option(ASSIMP_NO_EXPORT ON)
# option(ASSIMP_NO_EXPORT OFF)
# set(ASSIMP_BUILD_COLLADA_EXPORTER TRUE)
# set(ASSIMP_BUILD_FBX_EXPORTER TRUE)
# set(ASSIMP_BUILD_GLTF_EXPORTER TRUE)
# set(ASSIMP_BUILD_OBJ_EXPORTER TRUE)
# set(ASSIMP_BUILD_PLY_EXPORTER TRUE)
# set(ASSIMP_BUILD_STL_EXPORTER TRUE)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
option(ASSIMP_BUILD_ZLIB ON)

add_subdirectory(assimp)

# ----------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.18)
project(pyglsl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build Unit Tests" ON)

include(FetchContent)

FetchContent_Declare(glslang
    GIT_REPOSITORY   https://github.com/KhronosGroup/glslang.git
    GIT_TAG          16.2.0
)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL             OFF CACHE BOOL "" FORCE)
set(ENABLE_CTEST            OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING           OFF CACHE BOOL "" FORCE)
set(ENABLE_OPT              OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glslang)

# TODO: fix that main branch to whatever sounds good
FetchContent_Declare(spirv_cross
    GIT_REPOSITORY   https://github.com/KhronosGroup/SPIRV-Cross.git
    GIT_TAG          main
)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_SPVREMAPPER      OFF CACHE BOOL "" FORCE)
set(ENABLE_OPT              OFF CACHE BOOL "" FORCE)
set(BUILD_EXTERNAL          OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_STATIC      ON  CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SHARED      OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI         OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glslang spirv_cross)

add_library(glslang_nodes STATIC
    src/parse.cpp
    src/emit.cpp
    src/Node.cpp
)
target_include_directories(glslang_nodes PUBLIC
    ${glslang_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(glslang_nodes PRIVATE
    glslang
    glslang-default-resource-limits
    SPIRV
    MachineIndependent
    GenericCodeGen
    spirv-cross-core
    spirv-cross-glsl
)

add_subdirectory(bindings)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

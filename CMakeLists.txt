cmake_minimum_required(VERSION 3.18)
project(pyglslang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build Unit Tests" ON)

include(FetchContent)

FetchContent_Declare(glslang
    GIT_REPOSITORY  https://github.com/KhronosGroup/glslang.git
    GIT_TAG         16.2.0
)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL             OFF CACHE BOOL "" FORCE)
set(ENABLE_CTEST            OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING           OFF CACHE BOOL "" FORCE)
set(ENABLE_OPT              OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glslang)

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
)

add_subdirectory(bindings)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

cmake_minimum_required(VERSION 3.18)
project(pyglslang)

include(FetchContent)

find_package(Python COMPONENTS Interpreter Development REQUIRED)
if(DEFINED Python3_EXECUTABLE)
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE} CACHE FILEPATH "" FORCE)
endif()

# currently needs pybind11 pre-installed -> might also FetchContent that
execute_process(
    COMMAND python -c "import pybind11; print(pybind11.get_cmake_dir())"
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(pybind11 CONFIG REQUIRED)

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

pybind11_add_module(pyglslang
    module.cpp
    parse.cpp
    emit.cpp
    Node.cpp
)

target_include_directories(pyglslang PRIVATE
    ${glslang_SOURCE_DIR}
    ${glslang_SOURCE_DIR}/SPIRV
    ${pybind11_INCLUDE_DIRS}
)
target_link_libraries(pyglslang PRIVATE
    pybind11::module
    glslang
    SPIRV
    glslang-default-resource-limits
)
set_target_properties(pyglslang PROPERTIES
    CXX_STANDARD                        17
    RUNTIME_OUTPUT_DIRECTORY            "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY            "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_BINARY_DIR}"
)
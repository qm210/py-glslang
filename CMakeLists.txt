cmake_minimum_required(VERSION 3.18)
project(pyglslang)

find_package(Python COMPONENTS Interpreter Development REQUIRED)
execute_process(
    COMMAND python -c "import pybind11; print(pybind11.get_cmake_dir())"
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(pybind11 CONFIG REQUIRED)

set(GLSLANG_INSTALL_DIR "${CMAKE_SOURCE_DIR}/../repos/glslang/install")

add_library(pyglslang MODULE bindings.cpp)

target_include_directories(pyglslang PRIVATE
    "${GLSLANG_INSTALL_DIR}/include"
    ${pybind11_INCLUDE_DIRS}
)

file(GLOB GLSLANG_LIBS "${GLSLANG_INSTALL_DIR}/lib/*.lib")

target_link_libraries(pyglslang PRIVATE
    pybind11::module
    ${GLSLANG_LIBS}
)

pybind11_extension(pyglslang)
set_target_properties(pyglslang PROPERTIES CXX_STANDARD 17)
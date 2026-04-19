set(OUTPUT_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "Directory for .pyd/.so build")

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

pybind11_add_module(pyglsl module.cpp)

target_include_directories(pyglsl
    PUBLIC
    ${pybind11_INCLUDE_DIRS}
)
target_link_libraries(pyglsl
    PRIVATE
    pybind11::module
    glslang_nodes
)

message("OUTPUT " ${OUTPUT_DIR})
set_target_properties(pyglsl PROPERTIES
    CXX_STANDARD                        17
    RUNTIME_OUTPUT_DIRECTORY            "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY            "${OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG      "${OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE    "${OUTPUT_DIR}"
)

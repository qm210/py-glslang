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

pybind11_add_module(pyglslang
    module.cpp
)
target_include_directories(pyglslang PUBLIC
    ${pybind11_INCLUDE_DIRS}
)
target_link_libraries(pyglslang PRIVATE
    pybind11::module
    glslang_nodes
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

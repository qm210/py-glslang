include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.5.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(doctest)

add_executable(tests
    test_emit.cpp
    test_parse.cpp
)

target_link_libraries(tests
    PRIVATE
    glslang_nodes
    doctest::doctest_with_main
)

target_include_directories(tests
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

add_test(NAME test COMMAND tests)
#include "doctest.h"
#include "module.h"

TEST_CASE("emit qualified declaration") {
    auto src = R"(#version 450
        layout(location=0)out vec4 outColor0;
        void main() { }
    )";
    RootNode root = parse(src).root();
    std::string target = emitGlobals(root.globals);
    // whitespace as given by glslang getCompleteString()
    CHECK((target == "layout( location=0) out vec4 outColor0;\n"));
}

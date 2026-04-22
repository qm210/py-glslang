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


const std::string medium = R"(#version 450
layout(location=0)out vec4 outColor;
layout (location = 2) uniform float iTime;
const float speed = 0.1;
const vec3 c = vec3(1.,0.,-1.);
void main()
{
    outColor=vec4(0.5 * c.xyx,1.0);
    outColor.r = sin(speed * iTime);
})";

TEST_CASE("do not re-emit folded constants ") {
    Parsed target = parse(medium);
    RootNode root = parse(medium).root();
    printLogs(target.logs);
    auto check = emit(target.node);
    CHECK((root.globals.size() == 2));
}

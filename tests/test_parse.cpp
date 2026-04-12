#include "doctest.h"
#include "module.h"

TEST_CASE("parse some small shader") {
    std::string given = R"(#version 450
        out vec4 outColor;
        layout(location=1) out vec4 outWhatever;

        float hash12(vec2 p) {
            return fract(p.xy.x);
        }

        void main() {
        }
    )";
    RootNode target = parse(given).root();
    CHECK((target.children.size() == 1));
    CHECK((target.globals.size() == 1));
}

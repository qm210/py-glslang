#include "doctest.h"
#include "module.h"

TEST_CASE("parse some function") {
    auto given = R"(
        float hash12(vec2 p) {
            return fract(p.xy.x);
        }
    )";
    Parsed target = parse(given);
    CHECK((target.ast != nullptr));
}

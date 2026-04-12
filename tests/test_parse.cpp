#include "doctest.h"
#include "module.h"

TEST_CASE("parse some small shader") {
    std::string given = R"(#version 450
        out vec4 outColor;
        layout(location=1) out vec4 outWhatever;

        float bpm = 125.;

        float hash12(vec2 p) {
            return fract(p.xy.x);
        }

        void main() {
        }
    )";
    RootNode target = parse(given).root();
    CHECK((target.globals.size() == 3));
    auto global0 = target.globals[0]->data_if<DeclareNode>();
    CHECK((global0->name == "outColor"));
    CHECK((global0->typeName == "vec4"));
    CHECK((global0->storage == "out"));
    auto global1= target.globals[1]->data_if<DeclareNode>();
    CHECK((global1->name == "outWhatever"));
    CHECK((global1->typeName == "vec4"));
    CHECK((global1->storage == "out"));
    auto global2 = target.globals[2]->data_if<SymbolNode>();
    CHECK((global2->name == "bpm"));
    CHECK((global2->typeName == "float"));
    CHECK((global2->storage == ""));
    CHECK((target.children.size() == 2));
    auto child1 = target.children[0]->data_if<SequenceNode>();
    auto bpm = child1->statements[0]->data_if<DeclareNode>();
    CHECK((bpm->name == "bpm"));
    CHECK((bpm->typeName == "float"));
    CHECK((bpm->storage == "global"));
    auto bpmValue = bpm->init->data_if<ConstantNode>()->value;
    CHECK((bpmValue == "125.0"));
    auto child2 = target.children[1]->data_if<FunctionNode>();
    CHECK((child2->returnType == "void"));
    CHECK((child2->name == "main"));
}

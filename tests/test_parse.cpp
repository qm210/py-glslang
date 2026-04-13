#include "doctest.h"
#include "module.h"

const std::string minimal = R"(#version 450
    out vec4 outColor;
    layout(location=1) out vec4 outWhatever;

    float bpm = 125.;

    float hash12(vec2 p) {
        return fract(p.xy.x);
    }

    void main() {
    }
)";

TEST_CASE("parse minimal shader") {
    RootNode target = parse(minimal).root();
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

const std::string medium = R"(#version 450
layout(location=0)out vec4 outColor0;

layout (location = 22) uniform float iTime;
const float iZoomSpeed = 0.1;
layout (location = 20) uniform float iPumpAmont;

float bpm = 125.;
float spb;
float stepTime;
float nbeats;
float scale;
float hardBeats;
float syncTime;
vec2 uv0;

const vec3 c = vec3(1.,0.,-1.);
const float pi = 3.14159;

void mainImageA(out vec4 fragColor, vec2 fragCoord) {
    fragColor = c.xyzx;
}

void main()
{
    spb = 60./bpm;
    stepTime = mod(iTime+.5*spb, spb)-.5*spb;
    nbeats = (iTime-stepTime+.5*spb)/spb + smoothstep(-.2*spb, .2*spb, stepTime);
    scale = smoothstep(-.3*spb, 0., stepTime)*smoothstep(.3*spb, 0., stepTime);
    hardBeats = round((iTime-stepTime)/spb);
    syncTime = iZoomSpeed * iTime + iPumpAmont * nbeats;

    vec4 col=vec4(0.5,0.5,0.5,1.0);

    mainImageA(outColor0, gl_FragCoord.xy);
})";

TEST_CASE("parse medium shader") {
    Parsed target = parse(medium);
    RootNode root = parse(medium).root();
    printLogs(target.logs);
    auto check = emit(target.node);
    CHECK((root.globals.size() == 3));
}

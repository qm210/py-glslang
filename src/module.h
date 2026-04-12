#ifndef PYGLSLANG_MODULE_H
#define PYGLSLANG_MODULE_H

#include <memory>
#include <vector>
#include <string>
#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Include/intermediate.h>
#include <GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include "Node.h"

enum Stage {
    STAGE_VERT = 0,
    STAGE_TESC = 1,
    STAGE_TESE = 2,
    STAGE_GEOM = 3,
    STAGE_FRAG = 4,
    STAGE_COMP = 5
};

struct Parsed {
    bool ok = false;
    std::string info;
    std::string debug;
    NodePtr ast;
    std::vector<uint32_t> spirv;
};

Parsed parse(const std::string& source, Stage = STAGE_FRAG);

std::string emit(const NodePtr& node, int level = 0);

#endif //PYGLSLANG_MODULE_H

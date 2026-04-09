#ifndef PYGLSLANG_PARSED_H
#define PYGLSLANG_PARSED_H

#include <memory>
#include <vector>
#include <string>
#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Include/intermediate.h>
#include <GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

#include "enums.h"
#include "Node.h"

struct Parsed {
    bool ok = false;
    std::string info;
    std::string debug;
    std::shared_ptr<Node> ast;
    std::vector<uint32_t> spirv;
};

#endif //PYGLSLANG_PARSED_H

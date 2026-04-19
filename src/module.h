#ifndef PYGLSLANG_MODULE_H
#define PYGLSLANG_MODULE_H

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Include/intermediate.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include "Node.h"
#include "TraverseLog.h"

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
    NodePtr node;
    TraverseLogs logs;

    [[nodiscard]]
    const RootNode& root() const {
        auto *root = node->data_if<RootNode>();
        if (!root) {
            throw std::runtime_error("Parsing AST failed!\n\n" + info);
        }
        return *root;
    }
};

Parsed parse(const std::string& source, Stage = STAGE_FRAG);

std::string emit(const NodePtr&, int level = 0);
std::string emitGlobals(const NodePtrs&);

#endif //PYGLSLANG_MODULE_H

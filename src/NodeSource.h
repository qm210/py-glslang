#ifndef PYGLSLANG_NODESOURCE_H
#define PYGLSLANG_NODESOURCE_H

#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>

struct NodeSource {
    int line;
    int column;
    std::string code;

    static NodeSource find(TIntermNode* n, const glslang::TIntermediate& intermediate) {
        // this needs EShMsgDebugInfo to actually hold information
        const glslang::TSourceLoc& loc = n->getLoc();
        const auto& sources = intermediate.getSourceText();
        NodeSource src;
        src.line = loc.line;
        src.column = loc.column;
        if (sources.empty()) {
            return src;
        }
        std::istringstream ss(sources);
        std::string line;
        int current = 1;
        while (std::getline(ss, line)) {
            if (current++ == loc.line) {
                src.code = line;
                break;
            }
        }
        return src;
    }

    [[nodiscard]]
    std::string to_str() const {
        return std::to_string(line)
               + ":" + std::to_string(column)
               + ":\"" + code + "\"";
    }
};

#endif //PYGLSLANG_NODESOURCE_H

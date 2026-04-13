#ifndef PYGLSLANG_TRAVERSELOG_H
#define PYGLSLANG_TRAVERSELOG_H

#include <string>
#include <vector>
#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Include/intermediate.h>

struct TraverseLog {
    std::string_view kind;
    std::string what;
    glslang::TVisit visit;
    std::string complete;
    size_t stackSize;
    size_t stackTopSize;

    static std::string visit_str(const TraverseLog& log) {
        switch (log.visit) {
            case glslang::EvPreVisit:
                return "pre";
            case glslang::EvInVisit:
                return "in";
            case glslang::EvPostVisit:
                return "post";
            default:
                return "";
        }
    }

    std::string stack_str() {
        return std::to_string(stackSize)
               + "/" + std::to_string(stackTopSize);
    }
};

using TraverseLogs = std::vector<TraverseLog>;

#endif //PYGLSLANG_TRAVERSELOG_H

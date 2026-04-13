#ifndef PYGLSLANG_TRAVERSELOG_H
#define PYGLSLANG_TRAVERSELOG_H

#include <string>
#include <vector>
#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Include/intermediate.h>
#include <iomanip>
#include <iostream>

struct TraverseLog {
    std::string_view kind;
    std::string what;
    glslang::TVisit visit;
    std::string complete;
    std::string source;
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

    std::string stack_str() const {
        return std::to_string(stackSize)
               + "/" + std::to_string(stackTopSize);
    }
};

using TraverseLogs = std::vector<TraverseLog>;

static void printLogs(const TraverseLogs& logs) {
    for (const auto& log : logs) {
        std::cout << std::left
                  << std::setw(12) << log.kind
                  << std::setw(20) << log.what
                  << std::setw(8)  << TraverseLog::visit_str(log)
                  << std::setw(8) << log.source
                  << std::right
                  << std::setw(30) << log.complete
                  << std::setw(6)  << log.stack_str()
                  << '\n';
    }
}

#endif //PYGLSLANG_TRAVERSELOG_H

#ifndef PYGLSLANG_NODE_H
#define PYGLSLANG_NODE_H

#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Include/intermediate.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>

struct Node {
    std::string kind;
    std::string name;
    std::string type;
    int line = 0;
    std::vector<std::shared_ptr<Node>> children;

    uint32_t countTotal();

    const Node& child(int i = 0) const {
        return *children[i];
    };
};

std::shared_ptr<Node> simplify(const Node& node);
std::vector<std::shared_ptr<Node>> simplifyChildren(const Node& node);

#endif //PYGLSLANG_NODE_H

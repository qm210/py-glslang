#include "Node.h"

static inline bool canDrop(const Node& n) {
    static const std::unordered_set<std::string> noise = {
            "linker-objects", "scope"
    };
    return noise.count(n.kind) && n.children.empty();
}

static inline bool canFlatten(const Node& n) {
    static const std::unordered_set<std::string> flatten = {
            "sequence", "shader"
    };
    return flatten.count(n.name) || flatten.count(n.kind);
}

static inline std::basic_string<char> specifyKind(const Node& node) {
    if (node.kind != "Aggregate") {
        return node.kind;
    }
    if (node.name.empty()) {
        return "Block";
    }
    static const std::unordered_map<std::basic_string<char>, std::basic_string<char>> map = {
            { "if", "If" },
            { "sequence", "Block" },
            { "linker-objects", "Root" },
            { "params", "Params" },
            { "function", "Function" },
            { "linker-objects", "Root" }
    };
    auto it = map.find(node.name);
    if (it == map.end()) {
        return "Call";
    }
    return it->second;
}

std::shared_ptr<Node> simplify(const Node& node) {
    if (canDrop(node)) {
        return nullptr;
    }

    auto children = simplifyChildren(node);

    if ((node.kind == "Aggregate" && node.name.empty()) && children.size() == 1) {
        return children[0];
    }

    auto copy = std::make_shared<Node>();
    copy->kind = specifyKind(node);
    copy->name = node.name;
    copy->type = node.type;
    copy->line = node.line;
    copy->children = std::move(children);
    return copy;
}

std::vector<std::shared_ptr<Node>> simplifyChildren(const Node& node) {
    std::vector<std::shared_ptr<Node>> result;
    for (const auto& child : node.children) {
        if (canDrop(*child)) {
            continue;
        }
        if (canFlatten(*child)) {
            for (auto& grandchild : simplifyChildren(*child)) {
                result.push_back(grandchild);
            }
        }
        else {
            auto s = simplify(*child);
            if (s != nullptr) {
                result.push_back(s);
            }
        }
    }
    return result;
}

uint32_t Node::countTotal() {
    uint32_t count = 1;
    for (const auto& child : children) {
        count += child->countTotal();
    }
    return count;
}

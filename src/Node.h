#ifndef PYGLSLANG_NODE_H
#define PYGLSLANG_NODE_H

#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <memory>
#include <vector>
#include <string>
#include "NodeSource.h"

struct Node;
using NodePtr = std::shared_ptr<Node>;
using NodePtrs = std::vector<NodePtr>;

struct SymbolNode {
    std::string name;
    std::string typeName;
    std::string storage;
};

struct DeclareNode: SymbolNode {
    NodePtr init;
    std::string fullQualifier;
};

struct ConstantNode {
    std::string value;
};

struct BinaryNode {
    std::string op;
    NodePtr lhs;
    NodePtr rhs;
};

struct UnaryNode {
    std::string op;
    NodePtr operand;
    bool isPostfix;
    bool isBuiltin;
};

struct CallNode {
    std::string funcName;
    NodePtrs args;
};

struct ConstructNode {
    std::string typeName;
    NodePtrs args;
};

struct SequenceNode {
    NodePtrs statements;
};

struct FunctionParam {
    std::string type;
    std::string name;
    std::string storage;
};

struct FunctionParts {
    std::vector<FunctionParam> params;
    NodePtrs body;
};

struct FunctionNode {
    std::string returnType;
    std::string name;
    std::vector<FunctionParam> params;
    NodePtrs body;
};

struct IfNode {
    NodePtr condition;
    NodePtrs trueBranch;
    NodePtrs falseBranch;
};

struct CaseNode {
    NodePtr label;
    NodePtrs body;
};

struct SwitchNode {
    NodePtr condition;
    NodePtrs cases;
};

struct LoopNode {
    NodePtr condition;
    NodePtr increment;
    NodePtrs body;
    bool isDoWhile = false;
};

struct ReturnNode {
    NodePtr value;
};

struct BreakNode {};
struct ContinueNode {};
struct DiscardNode {};

struct RootNode {
    NodePtrs globals;
    NodePtrs children;
};

using NodeVariant = std::variant<
        SymbolNode,
        ConstantNode,
        DeclareNode,
        BinaryNode,
        UnaryNode,
        CallNode,
        ConstructNode,
        SequenceNode,
        FunctionNode,
        IfNode,
        SwitchNode,
        CaseNode,
        LoopNode,
        ReturnNode,
        BreakNode,
        ContinueNode,
        DiscardNode,
        RootNode>;

struct Node {
    NodeVariant data;
    NodeSource src;

    template<typename T>
    explicit Node(T&& v, NodeSource src = {})
        : data(std::forward<T>(v))
        , src(std::move(src))
        {}

    template<typename T, typename... Args>
    static NodePtr make(NodeSource src, Args&&... args) {
        return std::make_shared<Node>(
                T{std::forward<Args>(args)...},
                std::move(src)
        );
    }

    template<typename T>
    T* data_if() {
        return std::get_if<T>(&data);
    }

    std::string kind() const;
    std::string label() const;

    template <typename T> [[nodiscard]]
    bool is() const {
        return std::holds_alternative<T>(data);
    }

    template <typename T> [[nodiscard]]
    bool isnt() const {
        return !is<T>();
    }

    template <typename... Ts> [[nodiscard]]
    static bool only_of(const NodePtrs& vec) {
        if (vec.empty()) {
            return false;
        }
        return std::all_of(
                vec.begin(),
                vec.end(),
                [](const NodePtr& n) {
                    return (n->is<Ts>() || ...);
                });
    }
};

std::shared_ptr<Node> simplify(const NodePtr node);

template<typename T>
T* data_of(const NodePtr& node) {
    return node->data_if<T>();
}

template<typename T>
std::shared_ptr<T> ptr_to_data(const NodePtr& node) {
    return std::shared_ptr<T>(node, node->data_if<T>());
}

inline NodePtr first(NodePtrs vec) {
    return vec.empty() ? nullptr : vec.front();
}

inline void moveAfter(NodePtrs& target, NodePtrs&& nodes) {
    target.insert(
            target.end(),
            std::make_move_iterator(nodes.begin()),
            std::make_move_iterator(nodes.end())
    );
}

#endif //PYGLSLANG_NODE_H

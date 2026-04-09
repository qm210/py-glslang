#ifndef PYGLSLANG_NODE_H
#define PYGLSLANG_NODE_H

#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Include/intermediate.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>

struct Node;
using NodePtr = std::shared_ptr<Node>;

struct SymbolNode {
    std::string name;
    std::string typeName;
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
    bool postfix;
    NodePtr operand;
};

struct CallNode {
    std::string functionName;
    std::vector<NodePtr> args;
};

struct ConstructNode {
    std::string typeName;
    std::vector<NodePtr> args;
};

struct AssignNode {
    std::string op;
    NodePtr lhs;
    NodePtr rhs;
};

struct SequenceNode {
    std::vector<NodePtr> statements;
    bool isRoot;
};

using TypeNamePair = std::pair<std::string, std::string>;

struct FunctionNode {
    std::string returnType;
    std::string name;
    std::vector<TypeNamePair> params;
    std::vector<NodePtr> body;
};

struct IfNode {
    NodePtr condition;
    std::vector<NodePtr> trueBranch;
    std::vector<NodePtr> falseBranch;
};

struct WhileNode {
    NodePtr condition;
    std::vector<NodePtr> body;
    bool isDoWhile = false;
};

struct ReturnNode {
    NodePtr value;
};

struct BreakNode {};
struct ContinueNode {};
struct DiscardNode {};

using NodeVariant = std::variant<
        SymbolNode,
        ConstantNode,
        BinaryNode,
        UnaryNode,
        CallNode,
        ConstructNode,
        AssignNode,
        SequenceNode,
        FunctionNode,
        IfNode,
        WhileNode,
        ReturnNode,
        BreakNode,
        ContinueNode,
        DiscardNode
        >;

struct NodeSource {
    int line = -1;
    int column = -1;
    std::string code;
};

struct Node {
    NodeVariant data;
    NodeSource src;
    // for debugging:
    std::string kind;

    template<typename T>
    Node(T&& v, NodeSource src = {})
        : data(std::forward<T>(v))
        , src(std::move(src))
        {}

    template<typename T, typename... Args>
    static NodePtr make(Args&&... args) {
        auto node = T{std::forward<Args>(args)...};
        return std::make_shared<Node>(node);
    }

    uint32_t countTotal();
};

std::shared_ptr<Node> simplify(const Node& node);
std::vector<std::shared_ptr<Node>> simplifyChildren(const Node& node);

#endif //PYGLSLANG_NODE_H

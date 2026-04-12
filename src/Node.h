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
using NodePtrs = std::vector<NodePtr>;

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

struct DeclareNode {
    std::string typeName;
    std::string name;
    NodePtr value;
};

struct UnaryNode {
    std::string op;
    NodePtr operand;
    bool isPostfix;
    bool isBuiltin;
};

struct CallNode {
    std::string functionName;
    NodePtrs args;
};

struct ConstructNode {
    std::string typeName;
    NodePtrs args;
};

struct SequenceNode {
    NodePtrs statements;
};

using TypeNamePair = std::pair<std::string, std::string>;

struct FunctionParts {
    std::vector<TypeNamePair> params;
    NodePtrs body;
};

struct FunctionNode {
    std::string returnType;
    std::string name;
    std::vector<TypeNamePair> params;
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

struct NodeSource {
    int line;
    int column;
    std::string code;
};

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

    std::string kind() const {
        return std::visit([](auto&& n) -> std::string {
            using T = std::decay_t<decltype(n)>;
            if constexpr (std::is_same_v<T, SymbolNode>)
                return "Symbol";
            if constexpr (std::is_same_v<T, ConstantNode>)
                return "Constant";
            if constexpr (std::is_same_v<T, DeclareNode>)
                return "Declare";
            if constexpr (std::is_same_v<T, BinaryNode>)
                return "Binary";
            if constexpr (std::is_same_v<T, UnaryNode>)
                return "Unary";
            if constexpr (std::is_same_v<T, CallNode>)
                return "Call";
            if constexpr (std::is_same_v<T, ConstructNode>)
                return "Construct";
            if constexpr (std::is_same_v<T, SequenceNode>)
                return "Sequence";
            if constexpr (std::is_same_v<T, FunctionNode>)
                return "Function";
            if constexpr (std::is_same_v<T, IfNode>)
                return "If";
            if constexpr (std::is_same_v<T, SwitchNode>)
                return "Switch";
            if constexpr (std::is_same_v<T, CaseNode>)
                return "Case";
            if constexpr (std::is_same_v<T, LoopNode>)
                return "Loop";
            if constexpr (std::is_same_v<T, ReturnNode>)
                return "Return";
            if constexpr (std::is_same_v<T, BreakNode>)
                return "Break";
            if constexpr (std::is_same_v<T, ContinueNode>)
                return "Continue";
            if constexpr (std::is_same_v<T, DiscardNode>)
                return "Discard";
            if constexpr (std::is_same_v<T, RootNode>)
                return "Root";
            return "Unknown";
        }, data);
    }

    std::string label() const {
        return std::visit([](auto&& n) -> std::string {
            using T = std::decay_t<decltype(n)>;
            if constexpr (std::is_same_v<T, SymbolNode>)
                return n.name;
            if constexpr (std::is_same_v<T, ConstantNode>)
                return n.value;
            if constexpr (std::is_same_v<T, DeclareNode>)
                return n.name;
            if constexpr (std::is_same_v<T, BinaryNode>)
                return n.op;
            if constexpr (std::is_same_v<T, UnaryNode>)
                return n.op;
            if constexpr (std::is_same_v<T, CallNode>)
                return n.functionName;
            if constexpr (std::is_same_v<T, ConstructNode>)
                return n.typeName;
            if constexpr (std::is_same_v<T, SequenceNode>)
                return std::to_string(n.statements.size());
            if constexpr (std::is_same_v<T, FunctionNode>)
                return n.name;
            if constexpr (std::is_same_v<T, IfNode>)
                return n.condition->label();
            if constexpr (std::is_same_v<T, SwitchNode>)
                return n.condition->label();
            if constexpr (std::is_same_v<T, CaseNode>)
                return n.label->label();
            if constexpr (std::is_same_v<T, LoopNode>)
                return n.condition->label();
            return "";
        }, data);
    }

    template <typename T> [[nodiscard]]
    bool is() const {
        return std::holds_alternative<T>(data);
    }

    template <typename T> [[nodiscard]]
    bool isnt() const {
        return !is<T>();
    }

    template <typename T> [[nodiscard]]
    static bool all_are(const NodePtrs& vec) {
        if (vec.empty()) {
            return false;
        }
        return std::all_of(
                vec.begin(),
                vec.end(),
                [](const NodePtr& n) {
                    return n->is<T>();
                });
    }
};

std::shared_ptr<Node> simplify(const NodePtr node);

#endif //PYGLSLANG_NODE_H

#include "module.h"

static inline std::string indent(int level) {
    return std::string(4 * level, ' ');
}

bool needsSemicolon(const NodePtr& node) {
    return node->isnt<FunctionNode>()
        && node->isnt<SequenceNode>()
        && node->isnt<IfNode>()
        && node->isnt<LoopNode>()
        && node->isnt<SwitchNode>()
        && node->isnt<CaseNode>();
}

std::string emitList(const NodePtrs& nodes, const std::string& separator, int level) {
    std::string result;
    for (size_t i = 0; i < nodes.size(); i++) {
        if (i > 0) {
            result += separator;
        }
        result += emit(nodes[i], level);
    }
    return result;
}

std::string emitBlock(const NodePtrs& statements, int level) {
    std::string result;
    for (auto& each : statements) {
        result += indent(level) + emit(each, level);
        if (needsSemicolon(each)) {
            result += ";";
        }
        result += "\n";
    }
    return result;
}

template<typename T>
T* data_if(const NodePtr& node) {
    return node->data_if<T>();
}

std::string swizzleOf(const NodePtr& node) {
    // fixed set, because shader_minifier can switch afterwards
    static constexpr std::string_view swizzleSet = "xyzw";

    std::string res;
    auto appendIndex = [&res](const NodePtr& node) {
        if (auto *c = data_if<ConstantNode>(node)) {
            int index = std::stoi(c->value);
            if (index >= 0 && index < 4) {
                res += swizzleSet[index];
            }
        }
    };

    if (auto *seq = data_if<SequenceNode>(node)) {
        for (auto& s : seq->statements) {
            appendIndex(s);
        }
    } else {
        appendIndex(node);
    }
    return res;
}

std::string emit(const NodePtr& node, int level) {
    if (!node) {
        return "";
    }

    return std::visit([&](auto&& n) -> std::string {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, SymbolNode>) {
            return n.name;
        }
        if constexpr (std::is_same_v<T, ConstantNode>) {
            return n.value;
        }
        if constexpr (std::is_same_v<T, DeclareNode>) {
            std::string result = n.typeName + " " + n.name;
            if (n.value) {
                result += "=" + emit(n.value, level);
            }
            return result;
        }
        if constexpr (std::is_same_v<T, BinaryNode>) {
            if (n.op == "[]" || n.op == ".") {
                auto swizzle = swizzleOf(n.rhs);
                if (!swizzle.empty()) {
                    return emit(n.lhs, level) + "." + swizzle;
                }
                if (n.op == "[]") {
                    return emit(n.lhs, level) + "[" + emit(n.rhs, level) + "]";
                }
                return emit(n.lhs, level) + "." + emit(n.rhs, level);
            }
            return "(" + emit(n.lhs, level) + n.op + emit(n.rhs, level) + ")";
        }
        if constexpr (std::is_same_v<T, UnaryNode>) {
            if (n.isBuiltin) {
                return n.op + "(" + emit(n.operand, level) + ")";
            }
            if (n.isPostfix) {
                return "(" + emit(n.operand, level) + n.op + ")";
            }
            else {
                return "(" + n.op + emit(n.operand, level) + ")";
            }
        }
        if constexpr (std::is_same_v<T, CallNode>) {
            return n.functionName
                + "("
                + emitList(n.args, ", ", level)
                + ")";
        }
        if constexpr (std::is_same_v<T, ConstructNode>) {
            return n.typeName
                + "("
                + emitList(n.args, ", ", level)
                + ")";
        }
        if constexpr (std::is_same_v<T, SequenceNode>) {
            return emitBlock(n.statements, level);
        }
        if constexpr (std::is_same_v<T, FunctionNode>) {
            std::string params;
            for (size_t i = 0; i < n.params.size(); i++) {
                if (i > 0) {
                    params += ", ";
                }
                params += n.params[i].first + " " + n.params[i].second;
            }
            return n.returnType
                + " "
                + n.name
                + "("
                + params
                + ") {\n"
                + emitBlock(n.body, level + 1)
                + indent(level)
                + "}\n";
        }
        if constexpr (std::is_same_v<T, IfNode>) {
            std::string result =
                    "if (" + emit(n.condition, level) + ") {\n"
                    + emitBlock(n.trueBranch, level + 1)
                    + indent(level)
                    + "}";
            if (!n.falseBranch.empty()) {
                result += " else {\n"
                        + emitBlock(n.falseBranch, level + 1)
                        + indent(level)
                        + "}";
            }
            return result;
        }
        if constexpr (std::is_same_v<T, SwitchNode>) {
            return "switch (" + emit(n.condition, level) + ") {\n"
                    + emitBlock(n.cases, level + 1)
                    + indent(level)
                    + "}";
        }
        if constexpr (std::is_same_v<T, CaseNode>) {
            std::string label = n.label
                    ? ("case " + emit(n.label, level))
                    : "default";
            return label + ":\n"
                    + emitBlock(n.body, level + 1);
        }
        if constexpr (std::is_same_v<T, LoopNode>) {
            std::string body =
                    "{\n"
                    + emitBlock(n.body, level + 1)
                    + indent(level)
                    + "}";
            if (n.isDoWhile) {
                return "do "
                    + body
                    + " while ("
                    + emit(n.condition, level)
                    + ")";
            }
            if (n.increment) {
                return "for (; "
                    + emit(n.condition, level)
                    + "; "
                    + emit(n.increment, level)
                    + ") "
                    + body;
            }
            return "while ("
                + emit(n.condition, level)
                + ") "
                + body;
        }
        if constexpr (std::is_same_v<T, ReturnNode>) {
            return n.value
                ? ("return " + emit(n.value, level))
                : "return";
        }
        if constexpr (std::is_same_v<T, BreakNode>) {
            return "break";
        }
        if constexpr (std::is_same_v<T, ContinueNode>) {
            return "continue";
        }
        if constexpr (std::is_same_v<T, DiscardNode>) {
            return "discard";
        }
        if constexpr (std::is_same_v<T, RootNode>) {
            return emitBlock(n.globals, level)
                + "\n"
                + emitBlock(n.children, level);
        }
        return "/*" + node->src.code + "*/";
    }, node->data);
}

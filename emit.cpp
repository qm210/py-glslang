#include "module.h"

static inline std::string indent(int level) {
    return std::string(4 * level, ' ');
}

bool needsSemicolon(const NodePtr& node) {
    return !std::holds_alternative<IfNode>(node->data)
        && !std::holds_alternative<LoopNode>(node->data)
        && !std::holds_alternative<SwitchNode>(node->data)
        && !std::holds_alternative<CaseNode>(node->data);
}

std::string emitList(const std::vector<NodePtr>& nodes, const std::string& separator, int level) {
    std::string result;
    for (size_t i = 0; i < nodes.size(); i++) {
        if (i > 0) {
            result += separator;
        }
        result += emit(nodes[i], level);
    }
    return result;
}

std::string emitBlock(const std::vector<NodePtr>& statements, int level) {
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
            return "("
                + emit(n.lhs, level)
                + n.op
                + emit(n.rhs, level)
                + ")";
        }
        if constexpr (std::is_same_v<T, UnaryNode>) {
            if (n.postfix) {
                return "("
                    + emit(n.operand, level)
                    + n.op
                    + ")";
            }
            else {
                return "("
                    + n.op
                    + emit(n.operand, level)
                    + ")";
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
            std::string result = "switch (" + emit(n.condition, level) + ") {\n";
//                    + emitBlock(n.trueBranch, level + 1)
//                    + indent(level)
            result += "}";
            return result;
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
        return "/*" + node->src.code + "*/";
    }, node->data);
}

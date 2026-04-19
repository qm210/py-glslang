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

std::string emitArgs(const NodePtrs& nodes, int level) {
    return "(" + emitList(nodes, ", ", level) + ")";
}

std::string emitBlock(const NodePtrs& nodes, int level) {
    std::string result;
    for (auto& each : nodes) {
        result += indent(level) + emit(each, level);
        if (needsSemicolon(each)) {
            result += ";";
        }
        result += "\n";
    }
    return result;
}

std::string emitGlobals(const NodePtrs& nodes) {
    const int level = 0;
    std::string result;
    for (auto& each : nodes) {
        result += indent(level);
        if (auto *sym = each->data_if<SymbolNode>()) {
            result += sym->completeType + " " + sym->name;
        }
        else if (auto *decl = each->data_if<DeclareNode>()) {
            result += decl->completeType + " " + decl->name;
            if (decl->init) {
                result += " = " + emit(decl->init, level);
            }
        }
        result += ";\n";
    }
    return result;
}

std::string emitBody(const NodePtrs& nodes, int level) {
    return "{\n"
           + emitBlock(nodes, level + 1)
           + indent(level)
           + "}";
}

std::string swizzleOf(const NodePtr& node) {
    // fixed set, because shader_minifier can switch afterwards
    static constexpr std::string_view swizzleSet = "xyzw";

    std::string res;
    auto appendIndex = [&res](const NodePtr& node) {
        if (auto *c = data_of<ConstantNode>(node)) {
            int index = std::stoi(c->value);
            if (index >= 0 && index < 4) {
                res += swizzleSet[index];
            }
        }
    };

    if (auto *seq = data_of<SequenceNode>(node)) {
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
            if (n.init) {
                result += "=" + emit(n.init, level);
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
            return n.funcName + emitArgs(n.args, level);
        }
        if constexpr (std::is_same_v<T, ConstructNode>) {
            return n.typeName + emitArgs(n.args, level);
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
                const auto& param = n.params[i];
                if (!param.storage.empty()
                    && param.storage != "in") {
                    params += param.storage + " ";
                }
                params += param.type + " " + param.name;
            }
            return n.returnType + " " + n.name
                + "(" + params + ") "
                + emitBody(n.body, level) + "\n";
        }
        if constexpr (std::is_same_v<T, IfNode>) {
            std::string result =
                    "if (" + emit(n.condition, level) + ") "
                    + emitBody(n.trueBranch, level);
            if (!n.falseBranch.empty()) {
                result += " else "
                        + emitBody(n.falseBranch, level);
            }
            return result;
        }
        if constexpr (std::is_same_v<T, SwitchNode>) {
            return "switch (" + emit(n.condition, level) + ") "
                    + emitBody(n.cases, level);
        }
        if constexpr (std::is_same_v<T, CaseNode>) {
            std::string label = n.label
                    ? ("case " + emit(n.label, level))
                    : "default";
            return label + ":\n"
                    + emitBlock(n.body, level + 1);
        }
        if constexpr (std::is_same_v<T, LoopNode>) {
            std::string body = emitBody(n.body, level);
            std::string cond = emit(n.condition, level);
            if (n.isDoWhile) {
                return "do " + body
                    + " while (" + cond + ")";
            }
            if (n.increment) {
                std::string inc = emit(n.increment, level);
                return "for (; " + cond + "; " + inc + ") "
                    + body;
            }
            return "while (" + cond + ") " + body;
        }
        if constexpr (std::is_same_v<T, ReturnNode>) {
            if (n.value) {
                return "return " + emit(n.value, level);
            } else {
                return "return";
            }
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
            return emitGlobals(n.globals)
                + "\n"
                + emitBlock(n.children, level);
        }
        return "/*" + node->src.code + "*/";
    }, node->data);
}

#include "Node.h"

static NodePtrs simplify(const NodePtrs& nodes) {
    NodePtrs res;
    for (auto& n : nodes) {
        if (auto s = simplify(n)) {
            res.push_back(s);
        }
    }
    return res;
}

std::shared_ptr<Node> simplify(const NodePtr node) {
    if (!node) {
        return nullptr;
    }
    return std::visit([&](auto&& n) -> NodePtr {
        using T = std::decay_t<decltype(n)>;

        if constexpr (std::is_same_v<T, SequenceNode>) {
            NodePtrs simplified;
            for (auto& child : n.statements) {
                auto s = simplify(child);
                if (!s) continue;
                if (auto* sn = data_of<SequenceNode>(s)) {
                    for (auto& flat : sn->statements) {
                        simplified.push_back(flat);
                    }
                } else {
                    simplified.push_back(s);
                }
            }
            if (simplified.size() == 1) {
                return simplified[0];
            }
            return Node::make<SequenceNode>(node->src,
                                            std::move(simplified));
        }
        if constexpr (std::is_same_v<T, FunctionNode>) {
            NodePtrs body = simplify(n.body);
            return Node::make<FunctionNode>(node->src,
                                            n.returnType,
                                            n.name,
                                            n.params,
                                            std::move(body));
        }
        if constexpr (std::is_same_v<T, BinaryNode>) {
            return Node::make<BinaryNode>(node->src,
                                          n.op,
                                          simplify(n.lhs),
                                          simplify(n.rhs));
        }
        if constexpr (std::is_same_v<T, UnaryNode>) {
            return Node::make<UnaryNode>(node->src,
                                         n.op,
                                         simplify(n.operand),
                                         n.isPostfix,
                                         n.isBuiltin);
        }
        if constexpr (std::is_same_v<T, DeclareNode>) {
            return Node::make<DeclareNode>(node->src,
                                           n.name,
                                           n.typeName,
                                           n.storage,
                                           simplify(n.init),
                                           n.fullQualifier);
        }
        if constexpr (std::is_same_v<T, CallNode>) {
            NodePtrs args;
            for (auto& arg : n.args) {
                args.push_back(simplify(arg));
            }
            return Node::make<CallNode>(node->src,
                                        n.funcName,
                                        std::move(args));
        }
        if constexpr (std::is_same_v<T, ConstructNode>) {
            NodePtrs args = simplify(n.args);
            return Node::make<ConstructNode>(node->src,
                                             n.typeName,
                                             std::move(args));
        }
        if constexpr (std::is_same_v<T, IfNode>) {
            NodePtrs trueBranch = simplify(n.trueBranch);
            NodePtrs falseBranch = simplify(n.falseBranch);
            return Node::make<IfNode>(node->src,
                                      simplify(n.condition),
                                      std::move(trueBranch),
                                      std::move(falseBranch));
        }
        if constexpr (std::is_same_v<T, SwitchNode>) {
            NodePtrs cases;
            for (auto& s : n.cases) {
                auto& cn = std::get<CaseNode>(s->data);
                cases.push_back(
                        Node::make<CaseNode>(s->src,
                                             simplify(cn.label),
                                             simplify(cn.body))
                );
            }
            return Node::make<SwitchNode>(
                    node->src,
                    simplify(n.condition),
                    std::move(cases)
            );
        }
        if constexpr (std::is_same_v<T, LoopNode>) {
            return Node::make<LoopNode>(
                    node->src,
                    simplify(n.condition),
                    simplify(n.increment),
                    simplify(n.body),
                    n.isDoWhile
            );
        }
        if constexpr (std::is_same_v<T, ReturnNode>) {
            return Node::make<ReturnNode>(node->src,
                                          simplify(n.value));
        }
        if constexpr (std::is_same_v<T, RootNode>) {
            return Node::make<RootNode>(node->src,
                                        simplify(n.globals),
                                        simplify(n.children));
        }
        return node;
    }, node->data);    
}

std::string Node::kind() const {
    return std::visit([](auto&& n) -> std::string {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, DeclareNode>)
            return "Declare";
        if constexpr (std::is_same_v<T, SymbolNode>)
            return "Symbol";
        if constexpr (std::is_same_v<T, ConstantNode>)
            return "Constant";
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

std::string Node::label() const {
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
            return n.funcName;
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
        if constexpr (std::is_same_v<T, RootNode>)
            return "(root)";
        return "";
    }, data);
}

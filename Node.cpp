#include "Node.h"

std::shared_ptr<Node> simplify(const NodePtr node) {
    if (!node) {
        return nullptr;
    }
    return std::visit([&](auto&& n) -> NodePtr {
        using T = std::decay_t<decltype(n)>;

        if constexpr (std::is_same_v<T, SequenceNode>) {
            std::vector<NodePtr> simplified;
            for (auto& child : n.statements) {
                auto s = simplify(child);
                if (!s) continue;
                if (auto* inner = std::get_if<SequenceNode>(&s->data)) {
                    for (auto& flat : inner->statements)
                        simplified.push_back(flat);
                } else {
                    simplified.push_back(s);
                }
            }
            if (simplified.size() == 1) return simplified[0];
            return Node::make<SequenceNode>(node->src, std::move(simplified));
        }
        if constexpr (std::is_same_v<T, FunctionNode>) {
            std::vector<NodePtr> body;
            for (auto& stmt : n.body) {
                auto s = simplify(stmt);
                if (s) body.push_back(s);
            }
            return Node::make<FunctionNode>(node->src, n.returnType, n.name, n.params, std::move(body));
        }
        if constexpr (std::is_same_v<T, BinaryNode>) {
            return Node::make<BinaryNode>(node->src, n.op, simplify(n.lhs), simplify(n.rhs));
        }
        if constexpr (std::is_same_v<T, UnaryNode>) {
            return Node::make<UnaryNode>(node->src, n.op, simplify(n.operand), n.isPostfix, n.isBuiltin);
        }
        if constexpr (std::is_same_v<T, DeclareNode>) {
            return Node::make<DeclareNode>(node->src, n.typeName, n.name, simplify(n.value));
        }
        if constexpr (std::is_same_v<T, CallNode>) {
            std::vector<NodePtr> args;
            for (auto& arg : n.args) {
                args.push_back(simplify(arg));
            }
            return Node::make<CallNode>(node->src, n.functionName, std::move(args));
        }
        if constexpr (std::is_same_v<T, ConstructNode>) {
            std::vector<NodePtr> args;
            for (auto& arg : n.args) {
                args.push_back(simplify(arg));
            }
            return Node::make<ConstructNode>(node->src, n.typeName, std::move(args));
        }
        if constexpr (std::is_same_v<T, IfNode>) {
            std::vector<NodePtr> trueBranch;
            for (auto& s : n.trueBranch) {
                trueBranch.push_back(simplify(s));
            }
            std::vector<NodePtr> falseBranch;
            for (auto& s : n.falseBranch) {
                falseBranch.push_back(simplify(s));
            }
            return Node::make<IfNode>(
                    node->src,
                    simplify(n.condition),
                    std::move(trueBranch),
                    std::move(falseBranch)
            );
        }
        if constexpr (std::is_same_v<T, SwitchNode>) {
            std::vector<NodePtr> cases;
            for (auto& s : n.cases) {
                auto& caseNode = std::get<CaseNode>(s->data);
                std::vector<NodePtr> body;
                for (auto& b: caseNode.body) {
                    body.push_back(simplify(b));
                }
                cases.push_back(Node::make<CaseNode>(
                        s->src,
                        simplify(caseNode.label),
                        body
                ));
            }
            return Node::make<SwitchNode>(
                    node->src,
                    simplify(n.condition),
                    std::move(cases)
            );
        }
        if constexpr (std::is_same_v<T, LoopNode>) {
            std::vector<NodePtr> body;
            for (auto& s : n.body) body.push_back(simplify(s));
            return Node::make<LoopNode>(
                    node->src,
                    simplify(n.condition),
                    simplify(n.increment),
                    std::move(body),
                    n.isDoWhile
            );
        }
        if constexpr (std::is_same_v<T, ReturnNode>) {
            return Node::make<ReturnNode>(node->src, simplify(n.value));
        }
        return node;
    }, node->data);    
}

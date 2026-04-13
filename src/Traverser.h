#ifndef PYGLSLANG_TRAVERSER_H
#define PYGLSLANG_TRAVERSER_H

#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

#include "Node.h"
#include "NodeSource.h"
#include "parse_op.h"
#include "parse_type.h"
#include "parse_const.h"
#include "TraverseLog.h"

using namespace glslang;


struct Traverser : public TIntermTraverser {
private:
    std::vector<NodePtrs> stack;
    std::unordered_set<long long> knownIds;
    const TIntermediate& intermediate;
    TraverseLogs logs;

    NodePtrs& top() {
        return stack.back();
    }

    NodePtrs pop() {
        auto vec = std::move(top());
        stack.pop_back();
        return vec;
    }

    void pushStack() {
        stack.emplace_back();
    }

    template <typename T, typename... Args>
    void addNode(TIntermNode* n, Args&&... args) {
        NodePtr node = Node::make<T>(
                src(n), std::forward<Args>(args)...
        );
        top().push_back(node);
    }

    NodeSource src(TIntermNode* n) {
        return NodeSource::find(n, intermediate);
    }

    NodePtrs traverseChildren(TIntermNode* n) {
        if (!n) {
            return NodePtrs{};
        }
        pushStack();
        n->traverse(this);
        return pop();
    }

    void logVisit(std::string_view kind, TIntermNode *n, TVisit visit = EvPreVisit) {
        auto source = src(n);
        logs.push_back(TraverseLog{
                kind, "", visit, "",
                source.to_str(), stack.size(), top().size()
        });
    }

    void logVisit(std::string_view kind, TIntermTyped *n, TVisit visit = EvPreVisit) {
        auto source = src(n);
        logs.push_back(TraverseLog{
                kind, typeStr(n->getType()), visit,
                std::string(n->getType().getCompleteString(true)),
                source.to_str(), stack.size(), top().size()
        });
    }

    void logVisit(std::string_view kind, TIntermTyped *n, std::string name) {
        auto source = src(n);
        logs.push_back(TraverseLog{
                kind, typeStr(n->getType()) + " " + name, EvPreVisit,
                std::string(n->getType().getCompleteString(true)),
                source.to_str(), stack.size(), top().size()
        });
    }

    void logVisit(std::string_view kind, TIntermOperator *n, TVisit visit, std::string name = "") {
        auto source = src(n);
        logs.push_back(TraverseLog{
                kind, opStr(n->getOp()) + " " + name, visit,
                std::string(n->getType().getCompleteString(true)),
                source.to_str(), stack.size(), top().size()
        });
    }

public:
    explicit Traverser(const TIntermediate& intermediate)
        : TIntermTraverser(true, false, true, false, true)
        , intermediate(intermediate)
    {
        pushStack();
    }

    NodePtr build() {
        auto top = first(pop());
        if (!top) {
            return nullptr;
        }
        auto sn = top->data_if<SequenceNode>();
        if (!sn) {
            return nullptr;
        }
        RootNode node{};
        for (auto& child: sn->statements) {
            if (auto *cn = child->data_if<ConstructNode>()) {
                if (Node::only_of<SymbolNode, DeclareNode>(cn->args)) {
                    moveAfter(node.globals, std::move(cn->args));
                    continue;
                }
            }
            node.children.push_back(child);
        }
        return Node::make<RootNode>({}, std::move(node));
    }

    [[nodiscard]]
    const TraverseLogs& logged() const { return logs; }

    void visitSymbol(TIntermSymbol* n) override {
        logVisit("Symbol", n, std::string(n->getMangledName()));
        auto name = std::string(n->getName());
        const TType& t = n->getType();
        std::string storage(t.getStorageQualifierString());
        bool firstSeen = knownIds.insert(n->getId()).second;
        if (firstSeen) {
            addNode<DeclareNode>(
                    n,
                    name,
                    typeStr(t),
                    storage,
                    completeTypeStr(t),
                    nullptr
            );
        } else {
            addNode<SymbolNode>(
                    n,
                    name,
                    typeStr(t),
                    storage,
                    completeTypeStr(t)
            );
        }
    }

    void visitConstantUnion(TIntermConstantUnion* n) override {
        logVisit("Constant", n);
        addNode<ConstantNode>(
                n,
                formatConstant(n)
        );
    }

    /*
     * Note: the bool return value means (only for EvPreVisit)
     * true: automatically traverse the children afterwards
     * false: no automatic traversal, need to do manually then
     *        -> will also suppress EvPostVisit!
     */

    bool visitBinary(TVisit visit, TIntermBinary* n) override {
        logVisit("Binary", n, visit);
        if (visit == EvPreVisit) {
            pushStack();
            return true;
        }
        auto children = pop();
        if (auto decl = declarationSymbol(n)) {
            auto& type = decl->getType();
            addNode<DeclareNode>(
                    n,
                    std::string(decl->getName()),
                    typeStr(type),
                    type.getStorageQualifierString(),
                    completeTypeStr(type),
                    children[1]
            );
        } else {
            addNode<BinaryNode>(
                    n,
                    opStr(n->getOp()),
                    children[0],
                    children[1]
            );
        }
        return true;
    }

    bool visitUnary(TVisit visit, TIntermUnary* n) override {
        logVisit("Unary", n, visit);
        if (visit == EvPreVisit) {
            pushStack();
            return true;
        }
        auto children = pop();
        auto op = n->getOp();
        if (op == EOpConvNumeric) {
            auto typeName = typeStr(n->getType());
            addNode<ConstructNode>(
                    n,
                    typeName,
                    std::move(children)
            );
            return true;
        }
        addNode<UnaryNode>(
                n,
                opStr(op),
                children[0],
                op == EOpPostIncrement || op == EOpPostDecrement,
                builtinName(op) != nullptr
        );
        return true;
    }

    static std::string actualName(TIntermAggregate* n) {
        std::string s(n->getName());
        auto args = s.find('(');
        if (args != std::string::npos) {
            s.erase(args);
        }
        return s;
    }

    static FunctionParts splitFunction(const NodePtrs& children)
    {
        FunctionParts result;
        int index = 0;
        if (children.size() >= 2) {
            if (auto *seq = children[0]->data_if<SequenceNode>()) {
                for (auto& param : seq->statements) {
                    SymbolNode* sym = param->data_if<SymbolNode>();
                    if (auto* decl = param->data_if<DeclareNode>()) {
                        sym = static_cast<SymbolNode*>(decl);
                    }
                    if (!sym) {
                        continue;
                    }
                    result.params.push_back({
                        sym->typeName,
                        sym->name,
                        sym->storage,
                    });
                }
            }
            index = 1;
        }
        if (index < children.size()) {
            if (auto* seq = children[index]->data_if<SequenceNode>()) {
                result.body = seq->statements;
            } else {
                result.body = { children[index] };
            }
        }
        return result;
    }

    bool visitAggregate(TVisit visit, TIntermAggregate* n) override {
        logVisit("Aggregate", n, visit, std::string(n->getName()));
        if (visit == EvPreVisit) {
            pushStack();
            return true;
        }
        auto children = pop();
        switch (n->getOp()) {
            case EOpFunction: {
                auto name = actualName(n);
                auto [params, body] =
                        splitFunction(children);
                addNode<FunctionNode>(
                        n,
                        typeStr(n->getType()),
                        name,
                        std::move(params),
                        std::move(body)
                );
                break;
            }
            case EOpFunctionCall:
                addNode<CallNode>(
                        n,
                        actualName(n),
                        std::move(children)
                );
                break;
            case EOpParameters:
            case EOpSequence:
                addNode<SequenceNode>(
                        n,
                        std::move(children)
                );
                break;
            default:
                if (const char* builtin = builtinName(n->getOp())) {
                    addNode<CallNode>(
                            n,
                            std::string(builtin),
                            std::move(children)
                    );
                } else {
                    addNode<ConstructNode>(
                            n,
                            typeStr(n->getType()),
                            std::move(children)
                    );
                }
                break;
        }
        return true;
    }

    bool visitSelection(TVisit visit, TIntermSelection* n) override {
        logVisit("Aggregate", n, visit);
        if (visit != EvPreVisit) {
            return true;
        }
        auto condition = first(traverseChildren(n->getCondition()));
        auto trueBranch = traverseChildren(n->getTrueBlock());
        auto falseBranch = traverseChildren(n->getFalseBlock());
        addNode<IfNode>(
                n,
                condition,
                std::move(trueBranch),
                std::move(falseBranch)
        );
        return false;
    }

    bool visitSwitch(TVisit visit, TIntermSwitch* n) override {
        logVisit("Switch", n, visit);
        if (visit == EvPreVisit) {
            pushStack();
            return true;
        }
        auto children = pop();
        auto condition = std::move(children[0]);
        auto body = children[1]->data_if<SequenceNode>();
        auto seq = body->statements;
        NodePtrs cases;
        for (size_t i = 0; i < seq.size() - 1; i += 2) {
            auto caseNode = seq[i]->data_if<CaseNode>();
            auto bodyNode = seq[i+1]->data_if<SequenceNode>();
            caseNode->body = std::move(bodyNode->statements);
            cases.push_back(seq[i]);
        }
        addNode<SwitchNode>(
                n,
                std::move(condition),
                std::move(cases)
        );
        return false;
    }

    bool visitLoop(TVisit visit, TIntermLoop* n) override {
        logVisit("Loop", n, visit);
        if (visit != EvPreVisit) {
            return true;
        }
        NodePtr condition = first(traverseChildren(n->getTest()));
        NodePtr increment = first(traverseChildren(n->getTerminal()));
        auto body = traverseChildren(n->getBody());
        addNode<LoopNode>(
                n,
                condition,
                increment,
                std::move(body),
                !n->testFirst()
        );
        return false;
    }

    bool visitBranch(TVisit visit, TIntermBranch* n) override {
        logVisit("Branch", n, visit);
        if (visit != EvPreVisit) {
            return true;
        }
        auto value = first(traverseChildren(n->getExpression()));
        switch (n->getFlowOp()) {
            case EOpReturn:
                addNode<ReturnNode>(n, value);
                break;
            case EOpBreak:
                addNode<BreakNode>(n);
                break;
            case EOpContinue:
                addNode<ContinueNode>(n);
                break;
            case EOpKill:
                addNode<DiscardNode>(n);
                break;
            case EOpCase:
                addNode<CaseNode>(n, value);
                break;
            case EOpDefault:
                addNode<CaseNode>(n, nullptr);
                break;
            default:
                printf("Unhandled Branch FlowOp %d\n", n->getFlowOp());
                return true;
        }
        return false;
    }

};

/*
 * TODOS:
 * -- layout qualifiers (DeclareNode) appear in main() ?
 * -- type "int ..." also
 */

#endif //PYGLSLANG_TRAVERSER_H

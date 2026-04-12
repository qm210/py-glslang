#ifndef PYGLSLANG_TRAVERSER_H
#define PYGLSLANG_TRAVERSER_H

#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

#include "Node.h"
#include "NodeSource.h"
#include "parse_op.h"
#include "parse_type.h"
#include "parse_const.h"

using namespace glslang;

static bool isGlobal(TIntermTyped* n) {
    const auto& storage =
            n->getType().getQualifier().storage;
    return storage == EvqUniform
        || storage == EvqConst
        || storage == EvqGlobal
        || storage == EvqVaryingOut
        || storage == EvqVaryingIn;
}

struct Traverser : public TIntermTraverser {
private:
    std::vector<NodePtrs> stack;
    const TIntermediate& intermediate;

    NodePtrs& top() {
        return stack.back();
    }

    NodePtrs pop() {
        auto vec = std::move(top());
        stack.pop_back();
        return vec;
    }

    void pushStack() {
        stack.push_back({});
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

public:
    explicit Traverser(const TIntermediate& intermediate)
        : TIntermTraverser(true, false, true)
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

    void visitSymbol(TIntermSymbol* n) override {
        auto name = std::string(n->getName().c_str());
        if (isGlobal(n)) {
            const TType& t = n->getType();
            std::string storage(t.getStorageQualifierString());
            std::string full(t.getCompleteString(true));
            addNode<DeclareNode>(
                    n,
                    name,
                    typeStr(t),
                    storage,
                    nullptr, // values get initialized elsewhere
                    full
            );
        } else {
            addNode<SymbolNode>(
                    n,
                    name,
                    typeStr(n->getType())
            );
        }
    }

    void visitConstantUnion(TIntermConstantUnion* n) override {
        addNode<ConstantNode>(
                n,
                formatConstant(n)
        );
    }

    static TIntermSymbol* declarationSymbol(TIntermBinary* n) {
        if (n->getOp() != EOpAssign) {
            return nullptr;
        }
        auto *symbol = n->getLeft()->getAsSymbolNode();
        if (!symbol) {
            return nullptr;
        }
        auto storage = symbol->getType().getQualifier().storage;
        if (storage == EvqTemporary || storage == EvqGlobal) {
            return symbol;
        }
        return nullptr;
    }

    /*
     * Note: the bool return value means (only for EvPreVisit)
     * true: automatically traverse the children afterwards
     * false: no automatic traversal, need to do manually then
     *        -> will also suppress EvPostVisit!
     */

    bool visitBinary(TVisit visit, TIntermBinary* n) override {
        if (visit == EvPreVisit) {
            pushStack();
        } else if (visit == EvPostVisit) {
            auto children = pop();
            if (auto decl = declarationSymbol(n)) {
                auto& type = decl->getType();
                addNode<DeclareNode>(
                        n,
                        std::string(decl->getName()),
                        typeStr(type),
                        type.getStorageQualifierString(),
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
        }
        return true;
    }

    bool visitUnary(TVisit visit, TIntermUnary* n) override {
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

    static bool nodesEqual(const NodePtr& a, const NodePtr& b) {
        if (!a || !b)
            return a == b;
        if (a->data.index() != b->data.index())
            return false;
        if (auto* ca = a->data_if<ConstructNode>()) {
            auto* cb = b->data_if<ConstructNode>();
            if (ca->typeName != cb->typeName)
                return false;
            if (ca->args.size() != cb->args.size())
                return false;
            for (size_t i = 0; i < ca->args.size(); i++) {
                if (!nodesEqual(ca->args[i], cb->args[i]))
                    return false;
            }
            return true;
        }
        if (auto* ca = a->data_if<CallNode>()) {
            auto* cb = b->data_if<CallNode>();
            if (ca->funcName != cb->funcName)
                return false;
            if (ca->args.size() != cb->args.size())
                return false;
            for (size_t i = 0; i < ca->args.size(); i++) {
                if (!nodesEqual(ca->args[i], cb->args[i]))
                    return false;
            }
            return true;
        }
        if (auto* ca = a->data_if<SymbolNode>()) {
            return ca->name == b->data_if<SymbolNode>()->name;
        }
        if (auto* ca = a->data_if<ConstantNode>()) {
            return ca->value == b->data_if<ConstantNode>()->value;
        }
        return false;
    }

    static FunctionParts splitFunction(const NodePtrs& children)
    {
        FunctionParts result;
        int index = 0;
        if (children.size() >= 2) {
            if (auto *seq = children[0]->data_if<SequenceNode>()) {
                for (auto& param : seq->statements) {
                    if (auto* sym = param->data_if<SymbolNode>()) {
                        result.params.push_back({
                            sym->typeName,
                            sym->name,
                            sym->storage,
                        });
                    }
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
        if (visit != EvPreVisit) {
            printf("visitLoop ran into %d\n", visit);
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
 *
 *     vec2 off=vec2(mfnoise(vf2;f1;f1;f1;((uv+(iTime*vec2(1.0, 1.0))), 12.0, 1200.0, ca), mfnoise(vf2;f1;f1;f1;(((uv+(iTime*vec2(1.0, 1.0)))+1337.0), 12.0, 1200.0, ca));
 *
 *  and at very end:
 *
float(outColor0, outColor1, outColor2, outColor3, iResolution, iTime, iChannel0, iChannel1, iChannel2, iChannel3, iLastSwitchTime, iPalette, iFeedbackAmount, iCoordinates, iCoordinateScale, iCMAPScale, iCMAPOffset, iTrapOffset, iTrapParam, iTrapRadius, iTrap, iFormula, iOrigin, iConstantMapOffset, iFormulaMix, iOffset, iSampleCount, iJacobiRepeats, iPumpAmont, iZoomSpeed, iRotationSpeed, iBlendTime, iGlobalRotationSpeed, bpm, spb, stepTime, nbeats, scale, hardBeats, syncTime, uv0, c, pi, TRAP_TYPE_COUNT, FORMULA_COUNT, COORDINATES_JACOBI, COORDINATES_WEIERSTRASS, COORDINATES_MOEBIUS, COORDINATES_SINGLE_POLE, COORDINATES_SPIRAL_ZOOM);
 */

#endif //PYGLSLANG_TRAVERSER_H

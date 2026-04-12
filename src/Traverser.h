#ifndef PYGLSLANG_TRAVERSER_H
#define PYGLSLANG_TRAVERSER_H

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>

#include "Node.h"
#include "op_str.h"

using namespace glslang;

static std::string precisionQualifier(const TType& t) {
    switch (t.getQualifier().precision) {
        case EpqLow: return "lowp ";
        case EpqMedium: return "mediump ";
        case EpqHigh: return "highp ";
        default: return "";
    }
}

static std::string basicType(const TType& t) {
    switch (t.getBasicType()) {
        case EbtFloat: return "float";
        case EbtDouble: return "double";
        case EbtInt: return "int";
        case EbtUint: return "uint";
        case EbtBool: return "bool";
        case EbtVoid: return "void";
        default: return "";
    }
}

static std::string typePrefix(const TType& t) {
    switch (t.getBasicType()) {
        case EbtDouble: return "d";
        case EbtInt: return "i";
        case EbtUint: return "u";
        case EbtBool: return "b";
        default: return "";
    }
}

static inline const char oneDigit(int digit) {
    return '0' + digit;
}

static std::string typeStr(const TType& t) {
    std::string s = precisionQualifier(t);

    if (t.isStruct()) {
        s += t.getTypeName().c_str();
        return s;
    }
    else if (t.isMatrix()) {
        char cols = oneDigit(t.getMatrixCols());
        char rows = oneDigit(t.getMatrixRows());
        s += typePrefix(t) + "mat" + cols;
        if (cols != rows) {
            s += 'x' + rows;
        }
        return s;
    }
    else if (t.isVector()) {
        s += typePrefix(t) + "vec" + oneDigit(t.getVectorSize());
        return s;
    }

    std::string base = basicType(t);
    if (base.empty()) {
        return s + std::string(t.getCompleteString().c_str());
    } else {
        s += base;
    }

    if (t.isArray()) {
        for (int i = 0; i < (int)t.getArraySizes()->getNumDims(); ++i) {
            int dim = t.getArraySizes()->getDimSize(i);
            if (dim == 0) {
                s += "[]";
            } else {
                s += "[" + std::to_string(dim) + "]";
            }
        }
    }

    return s;
}

static std::string formatConstant(TIntermConstantUnion* n) {
    const TConstUnionArray& c = n->getConstArray();
    const TType& t = n->getType();

    auto scalar = [&](int i) -> std::string {
        switch (t.getBasicType()) {
            case EbtFloat: {
                double v = c[i].getDConst();
                std::ostringstream ss;
                ss << std::setprecision(8) << v;
                std::string s = ss.str();
                if (s.find('.') == std::string::npos
                    && s.find('e') == std::string::npos) {
                    s += ".0";
                }
                return s;
            }
            case EbtDouble: {
                std::ostringstream ss;
                ss << std::setprecision(17) << c[i].getDConst() << "lf";
                return ss.str();
            }
            case EbtInt:
                return std::to_string(c[i].getIConst());
            case EbtUint:
                return std::to_string(c[i].getUConst()) + "u";
            case EbtBool:
                return c[i].getBConst() ? "true" : "false";
            default:
                return "?";
        }
    };

    if (t.isScalar()) {
        return scalar(0);
    }

    int count = t.isMatrix()
            ? t.getMatrixCols() * t.getMatrixRows()
            : t.getVectorSize();
    std::string s = typeStr(t) + "(";
    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            s += ", ";
        }
        s += scalar(i);
    }
    s += ")";
    return s;
}

static NodeSource extract(TIntermNode* n, const TIntermediate& intermediate) {
    const TSourceLoc& loc = n->getLoc();
    NodeSource src;
    src.line = loc.line;
    src.column = loc.column;
    const auto& sources = intermediate.getSourceText();
    if (!sources.empty()) {
        std::istringstream ss(sources);
        std::string line;
        int current = 1;
        while (std::getline(ss, line)) {
            if (current++ == loc.line) {
                src.code = line;
                break;
            }
        }
    }
    return src;
}

struct Traverser : public TIntermTraverser {
private:
    std::vector<NodePtrs> stack;
    const TIntermediate& intermediate;

    NodePtrs& top() {
        return stack.back();
    }

    void pushStack() {
        stack.push_back({});
    }

    template <typename T, typename... Args>
    void addNode(TIntermNode* n, Args&&... args) {
        auto node = Node::make<T>(src(n), std::forward<Args>(args)...);
        top().push_back(node);
    }

    NodePtrs pop() {
        auto v = std::move(stack.back());
        stack.pop_back();
        return v;
    }

    NodeSource src(TIntermNode* n) {
        return extract(n, intermediate);
    }

    NodePtrs traverseChildren(TIntermNode* n) {
        if (!n) {
            return NodePtrs{};
        }
        pushStack();
        n->traverse(this);
        return pop();
    }

    NodePtr first(NodePtrs vec) {
        return vec.empty() ? nullptr: vec.front();
    }

public:
    explicit Traverser(const TIntermediate& intermediate)
        : TIntermTraverser(true, false, true)
        , intermediate(intermediate)
    {
        pushStack();
    }

    NodePtr root() {
        auto seq = pop();
        bool globalsFound = false;
        RootNode node{};
        for (auto& child: seq) {
            if (auto *cn = child->data_if<ConstructNode>()) {
                if (Node::all_are<SymbolNode>(cn->args)) {
                    // should only happen once
                    if (globalsFound) {
                        fprintf(stderr, "globals found twice! shouldn't be.\n");
                    } else {
                        node.globals = std::move(cn->args);
                    }
                    globalsFound = true;
                    continue;
                }
            }
            node.children.push_back(std::move(child));
        }
        return Node::make<RootNode>({}, std::move(node));
    }

    void visitSymbol(TIntermSymbol* n) override {
        addNode<SymbolNode>(
                n,
                std::string(n->getName().c_str()),
                typeStr(n->getType())
        );
    }

    void visitConstantUnion(TIntermConstantUnion* n) override {
        addNode<ConstantNode>(
                n,
                formatConstant(n)
        );
    }

    TIntermSymbol* declarationSymbol(TIntermBinary* n) {
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

    bool visitBinary(TVisit visit, TIntermBinary* n) override {
        if (visit == EvPreVisit) {
            pushStack();
        } else if (visit == EvPostVisit) {
            auto children = pop();
            if (auto declSymbol = declarationSymbol(n)) {
                addNode<DeclareNode>(
                        n,
                        typeStr(declSymbol->getType()),
                        std::string(declSymbol->getName()),
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

    /*
     * Note: the bool return value means (only for EvPreVisit)
     * true: automatically traverse the children afterwards
     * false: no automatic traversal, need to do manually then
     *        -> will also suppress EvPostVisit!
     */

    bool visitUnary(TVisit visit, TIntermUnary* n) override {
        if (visit == EvPreVisit) {
            pushStack();
        } else if (visit == EvPostVisit) {
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
        }
        return true;
    }

    static std::string baseName(const char* mangledName) {
        std::string s(mangledName);
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
            if (ca->functionName != cb->functionName)
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
                            sym->typeName, sym->name
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
        } else if (visit == EvPostVisit) {
            auto children = pop();
            switch (n->getOp()) {
                case EOpFunction: {
                    auto name = baseName(n->getName().c_str());
                    printf("EOpFunction %s children:%zu\n", name.c_str(), children.size());
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
                            std::string(n->getName().c_str()),
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

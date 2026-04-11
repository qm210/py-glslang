#ifndef PYGLSLANG_TRAVERSER_H
#define PYGLSLANG_TRAVERSER_H

#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>

#include "Node.h"

using namespace glslang;

static const char* builtinName(TOperator op) {
    switch (op) {
        case EOpMod: return "mod";
        case EOpModf: return "modf";
        case EOpSin: return "sin";
        case EOpCos: return "cos";
        case EOpTan: return "tan";
        case EOpAsin: return "asin";
        case EOpAcos: return "acos";
        case EOpAtan: return "atan";
        case EOpSqrt: return "sqrt";
        case EOpInverseSqrt: return "inversesqrt";
        case EOpAbs: return "abs";
        case EOpSign: return "sign";
        case EOpFloor: return "floor";
        case EOpCeil: return "ceil";
        case EOpFract: return "fract";
        case EOpPow: return "pow";
        case EOpExp: return "exp";
        case EOpExp2: return "exp2";
        case EOpLog: return "log";
        case EOpLog2: return "log2";
        case EOpMin: return "min";
        case EOpMax: return "max";
        case EOpClamp: return "clamp";
        case EOpMix: return "mix";
        case EOpStep: return "step";
        case EOpSmoothStep: return "smoothstep";
        case EOpLength: return "length";
        case EOpDistance: return "distance";
        case EOpDot: return "dot";
        case EOpCross: return "cross";
        case EOpNormalize: return "normalize";
        case EOpReflect: return "reflect";
        case EOpRefract: return "refract";
        case EOpFaceForward: return "faceforward";
        case EOpTexture: return "texture";
        case EOpTextureProj: return "textureProj";
        case EOpTextureLod: return "textureLod";
        case EOpTextureFetch: return "texelFetch";
        case EOpOuterProduct: return "outerProduct";
        case EOpTranspose: return "transpose";
        case EOpDeterminant: return "determinant";
        case EOpMatrixInverse: return "inverse";
        case EOpAll: return "all";
        case EOpAny: return "any";
        default: return nullptr;
    }
}

static std::string opStr(TOperator op) {
    switch (op) {
        case EOpAssign: return "=";
        case EOpAddAssign: return "+=";
        case EOpSubAssign: return "-=";
        case EOpMulAssign: return "*=";
        case EOpDivAssign: return "/=";
        case EOpAdd: return "+";
        case EOpSub: return "-";
        case EOpMul: return "*";
        case EOpDiv: return "/";
        case EOpNegative: return "-";
        case EOpEqual: return "==";
        case EOpNotEqual: return "!=";
        case EOpLessThan: return "<";
        case EOpGreaterThan: return ">";
        case EOpLessThanEqual: return "<=";
        case EOpGreaterThanEqual: return ">=";
        case EOpLogicalAnd: return "&&";
        case EOpLogicalOr: return "||";
        case EOpLogicalNot: return "!";
        case EOpVectorSwizzle: return ".";
        case EOpIndexDirect:
        case EOpIndexIndirect: return "[]";
        case EOpIndexDirectStruct: return ".";
        default: {
            if (const char* builtin = builtinName(op)) {
                return builtin;
            }
            return "op(" + std::to_string((int)op) + ")";
        }
    }
}

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
    std::vector<std::vector<NodePtr>> stack;
    const TIntermediate& intermediate;

    std::vector<NodePtr>& top() {
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

    std::vector<NodePtr> pop() {
        auto v = std::move(stack.back());
        stack.pop_back();
        return v;
    }

    NodeSource src(TIntermNode* n) {
        return extract(n, intermediate);
    }

    std::vector<NodePtr> traverseChildren(TIntermNode* n) {
        if (!n) {
            return std::vector<NodePtr>{};
        }
        pushStack();
        n->traverse(this);
        return pop();
    }

    NodePtr first(std::vector<NodePtr> vec) {
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
        NodePtr node = first(seq);
        return std::move(node);
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

    bool visitUnary(TVisit visit, TIntermUnary* n) override {
        if (visit == EvPreVisit) {
            pushStack();
        } else if (visit == EvPostVisit) {
            auto children = pop();
            auto op = n->getOp();
            addNode<UnaryNode>(
                    n,
                    opStr(op),
                    op == EOpPostIncrement || op == EOpPostDecrement,
                    children[0]
            );
        }
        return true;
    }

    static std::string baseName(const char* mangledName) {
        std::string s(mangledName);
        auto parent = s.find('(');
        if (parent != std::string::npos) {
            s.erase(parent);
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

    static std::vector<NodePtr> stripReturnTemporaries(std::vector<NodePtr> body) {
        // Some GLSL quirk duplicates return nodes as additional node. Do not want.
        for (size_t i = 1; i < body.size(); i++) {
            auto* ret = body[i - 1]->data_if<ReturnNode>();
            if (!ret) {
                continue;
            }
            if (nodesEqual(ret->value, body[i])) {
                body.erase(body.begin() + i);
                i--;
            }
        }
        return body;
    }

    static FunctionParts splitFunction(const std::vector<NodePtr>& children)
    {
        FunctionParts result;
        int bodyIndex = 0;
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
            bodyIndex = 1;
        }
        if (bodyIndex < children.size()) {
            if (auto* seq = children[bodyIndex]->data_if<SequenceNode>()) {
                result.body = seq->statements;
            } else {
                result.body = { children[bodyIndex] };
            }
        }
        result.body = stripReturnTemporaries(std::move(result.body));
        return result;
    }

    void dumpSequence(TIntermAggregate *n) {
        for (auto* child : n->getSequence()) {
            auto* childAgg = child->getAsAggregate();
            if (childAgg) {
                printf("  child op=%d (%s) type=%s\n",
                       childAgg->getOp(),
                       opStr(childAgg->getOp()).c_str(),
                       typeStr(childAgg->getType()).c_str());
            } else if (child->getAsSymbolNode()) {
                printf("  child symbol=%s\n",
                       child->getAsSymbolNode()->getName().c_str());
            } else if (child->getAsBranchNode()) {
                printf("  child branch op=%d\n",
                       child->getAsBranchNode()->getFlowOp());
            } else if (child->getAsUnaryNode()) {
                printf("  child unary op=%d\n",
                       child->getAsUnaryNode()->getOp());
            } else if (child->getAsBinaryNode()) {
                printf("  child binary op=%d (%s)\n",
                       child->getAsBinaryNode()->getOp(),
                       opStr(child->getAsBinaryNode()->getOp()).c_str());
            } else {
                printf("  child unknown type\n");
            }
        }
    }

    bool visitAggregate(TVisit visit, TIntermAggregate* n) override {
        if (visit == EvPreVisit) {
            pushStack();
        } else if (visit == EvPostVisit) {
            auto children = pop();
            switch (n->getOp()) {
                case EOpFunction: {
                    auto [params, body] =
                            splitFunction(children);
                    addNode<FunctionNode>(
                            n,
                            typeStr(n->getType()),
                            baseName(n->getName().c_str()),
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
                    dumpSequence(n);
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
        if (visit != EvPreVisit) {
            return true;
        }
        auto condition = first(traverseChildren(n->getCondition()));
        pushStack();
        for (auto *caseNode : n->getBody()->getSequence()) {
            caseNode->traverse(this);
        }
        auto cases = pop();
        addNode<SwitchNode>(
                n,
                condition,
                std::move(cases)
        );
        return false;
    }

    bool visitLoop(TVisit visit, TIntermLoop* n) override {
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
                printf("Ignore Branch FlowOp %d\n", n->getFlowOp());
                break;
        }
        return true;
    }

};

#endif //PYGLSLANG_TRAVERSER_H

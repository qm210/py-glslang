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
        case EOpMod: return "%";
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
        case EOpFunctionCall: return "call";
        case EOpFunction: return "function";
        case EOpParameters: return "params";
        case EOpLinkerObjects: return "linker-objects";
        case EOpSequence: return "sequence";
        case EOpScope: return "scope";
        case EOpVectorSwizzle: return "swizzle";
        case EOpIndexDirect: return "index";
        case EOpIndexIndirect: return "index[]";
        case EOpIndexDirectStruct: return "field";
        case EOpDot: return "dot";
        case EOpCross: return "cross";
        case EOpNormalize: return "normalize";
        case EOpLength: return "length";
        case EOpDistance: return "distance";
        case EOpReflect: return "reflect";
        case EOpRefract: return "refract";
        case EOpMix: return "mix";
        case EOpClamp: return "clamp";
        case EOpSmoothStep: return "smoothstep";
        case EOpStep: return "step";
        case EOpMin: return "min";
        case EOpMax: return "max";
        case EOpPow: return "pow";
        case EOpExp: return "exp";
        case EOpExp2: return "exp2";
        case EOpLog: return "log";
        case EOpLog2: return "log2";
        case EOpSqrt: return "sqrt";
        case EOpInverseSqrt: return "inversesqrt";
        case EOpAbs: return "abs";
        case EOpSign: return "sign";
        case EOpFloor: return "floor";
        case EOpCeil: return "ceil";
        case EOpFract: return "fract";
        case EOpSin: return "sin";
        case EOpCos: return "cos";
        case EOpTan: return "tan";
        case EOpAsin: return "asin";
        case EOpAcos: return "acos";
        case EOpAtan: return "atan";
        case EOpTexture: return "texture";
        case EOpTextureProj: return "textureProj";
        case EOpTextureLod: return "textureLod";
        case EOpTextureFetch: return "texelFetch";
        case EOpReturn: return "return";
        case EOpBreak: return "break";
        case EOpContinue: return "continue";
        case EOpTerminateInvocation: return "discard";
        default: return "op(" + std::to_string((int)op) + ")";
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

    std::vector<NodePtr>& top() {
        return stack.back();
    }

    void push() {
        stack.push_back({});
    }

    std::vector<NodePtr> pop() {
        auto v = std::move(stack.back());
        stack.pop_back();
        return v;
    }

    /*
    std::vector<NodePtr> stack;

    void pop() {
        stack.pop_back();
    }

    void push(Node* node) {
        stack.push_back(node);
    }

    void addToCurrent(std::shared_ptr<Node> child) {
        Node *top = stack.back();
        top->children.push_back(child);
    }

    Node* createChild(const std::basic_string<char> &kind,
                      const std::basic_string<char> &name,
                      const std::basic_string<char> &type,
                      int line) {
        auto node = std::make_shared<Node>();
        node->kind = kind;
        node->name = name;
        node->type = type;
        node->line = line;
        addToCurrent(node);
        return node.get();
    }
    */

    const TIntermediate& intermediate_;

    NodeSource src(TIntermNode* n) {
        return extract(n, intermediate_);
    }

public:
    // std::shared_ptr<Node> root;

    explicit Traverser(const TIntermediate& intermediate)
        : TIntermTraverser(true, false, true)
        , intermediate_(intermediate)
    {
//        root = std::make_shared<Node>(
//                SequenceNode{ .isRoot = true }
//        );
        push();
    }

    NodePtr root() {
        auto seq = pop();
        return Node::make<SequenceNode>(std::move(seq));
    }

    // ---- Symbols & constants ------------------------------------------

    void visitSymbol(TIntermSymbol* n) override {
        top().push_back(Node::make<SymbolNode>(
                std::string(n->getName().c_str()),
                typeStr(n->getType())
        ));
    }

    void visitConstantUnion(TIntermConstantUnion* n) override {
        top().push_back(Node::make<ConstantNode>(formatConstant(n)));
    }

    // ---- Binary -------------------------------------------------------

    bool visitBinary(TVisit visit, TIntermBinary* n) override {
        if (visit == EvPreVisit) {
            push(); // will collect [lhs, rhs]
        } else if (visit == EvPostVisit) {
            auto children = pop();
            // children[0] = lhs, children[1] = rhs
            auto node = Node::make<BinaryNode>(opStr(n->getOp()), children[0], children[1]);
            top().push_back(node);
        }
        return true;
    }

    // ---- Unary --------------------------------------------------------

    bool visitUnary(TVisit visit, TIntermUnary* n) override {
        if (visit == EvPreVisit) {
            push();
        } else if (visit == EvPostVisit) {
            auto children = pop();
            auto op = n->getOp();
            top().push_back(Node::make<UnaryNode>(
                    opStr(op),
                    op == EOpPostIncrement || op == EOpPostDecrement,
                    children[0]
            ));
        }
        return true;
    }

    // ---- Aggregates ---------------------------------------------------

    static std::string baseName(const char* mangledName) {
        std::string s(mangledName);
        auto paren = s.find('(');
        if (paren != std::string::npos)
            s.erase(paren);
        return s;
    }

    struct FunctionParts {
        std::vector<std::pair<std::string, std::string>> params; // {typeName, varName}
        std::vector<NodePtr> body;
    };

    static FunctionParts splitFunctionChildren(
            TIntermAggregate* funcNode,
            const std::vector<NodePtr>& children)
    {
        FunctionParts result;

        // children[0] is the EOpParameters node → a SequenceNode of SymbolNodes
        // children[1] is the EOpSequence body   → a SequenceNode of statement nodes
        // (if the function has no parameters, glslang may omit children[0])

        int bodyIndex = 0;

        if (children.size() >= 2) {
            // First child: parameters
            if (auto* seq = std::get_if<SequenceNode>(&children[0]->data)) {
                for (auto& param : seq->statements) {
                    if (auto* sym = std::get_if<SymbolNode>(&param->data)) {
                        result.params.push_back({ sym->typeName, sym->name });
                    }
                }
            }
            bodyIndex = 1;
        }

        // Remaining child: body sequence — unwrap the SequenceNode
        if (bodyIndex < (int)children.size()) {
            if (auto* seq = std::get_if<SequenceNode>(&children[bodyIndex]->data)) {
                result.body = seq->statements; // flat vector of statement NodePtrs
            } else {
                // Degenerate case: single-statement body not wrapped in a sequence
                result.body = { children[bodyIndex] };
            }
        }

        return result;
    }

    bool visitAggregate(TVisit visit, TIntermAggregate* n) override {
        if (visit == EvPreVisit) {
            push();
        } else if (visit == EvPostVisit) {
            auto children = pop();
            NodePtr result;

            switch (n->getOp()) {
                case EOpFunction: {
                    auto [params, body] = splitFunctionChildren(n, children);
                    result = Node::make<FunctionNode>(
                            typeStr(n->getType()),
                            baseName(n->getName().c_str()),
                            std::move(params),
                            std::move(body)   // ← this is what you reorder
                    );
                    break;
                }
                case EOpFunctionCall:
                    result = Node::make<CallNode>(
                            std::string(n->getName().c_str()),
                            std::move(children)
                    );
                    break;
                case EOpSequence:
                    result = Node::make<SequenceNode>(std::move(children));
                    break;
                default:
                    // constructors: vec4(...), mat3(...), etc.
                    result = Node::make<ConstructNode>(typeStr(n->getType()), std::move(children));
                    break;
            }
            top().push_back(result);
        }
        return true;
    }

    // ---- Selection, loops, branches: same pattern --------------------
    // (omitted for brevity — same push/pop/make<> approach)

/*

    Node* createChild(const std::basic_string<char> &kind,
               const std::basic_string<char> &name,
               const TIntermTyped *n) {
        return createChild(kind,
                    name,
                    typeStr(n->getType()),
                    n->getLoc().line);
    }

    Node* createChild(const std::basic_string<char> &kind,
               const TIntermOperator *n) {
        return createChild(kind, opStr(n->getOp()), n);
    }

    static std::basic_string<char> typeStr(const TType& t) {
        return t.getCompleteString().c_str();
    }

    template<typename T>
    bool visitNode(TVisit visit, const std::string& kind, T* n) {
        if (visit == EvPreVisit) {
            push(createChild(kind, n));
        }
        else if (visit == EvPostVisit) {
            pop();
        }
        return true;
    }

    bool visitNode(TVisit visit,
                   const std::string& kind,
                   const std::string& name,
                   const std::string& type,
                   int line) {
        if (visit == EvPreVisit) {
            push(createChild(kind, name, type, line));
        }
        else if (visit == EvPostVisit) {
            pop();
        }
        return true;
    }

    void visitSymbol(TIntermSymbol* n) override {
        createChild("Symbol", n->getName().c_str(), n);
    }

    void visitConstantUnion(TIntermConstantUnion* n) override {
        const auto& arr = n->getConstArray();
        auto u = arr[0];
        std::string name;
        switch (n->getType().getBasicType()) {
            case EbtFloat:
                name = std::to_string(u.getDConst());
                break;
            case EbtInt:
                name = std::to_string(u.getIConst());
                break;
            case EbtUint:
                name = std::to_string(u.getUConst());
                break;
            case EbtBool:
                name = u.getBConst() ? "true" : "false";
                break;
            default:
                name = "?";
                break;
        }
        createChild("Constant", name, n);
    }

    bool visitBinary(TVisit visit, TIntermBinary* n) override {
        return visitNode(visit, "Binary", n);
    }

    bool visitUnary(TVisit visit, TIntermUnary* n) override {
        return visitNode(visit, "Unary", n);
    }

    bool visitAggregate(TVisit visit, TIntermAggregate* n) override {
        if (visit == EvPreVisit) {
            std::string name(n->getName());
            if (name.empty()) {
                name = opStr(n->getOp());
            }
            push(createChild("Aggregate", name, n));
        }
        else if (visit == EvPostVisit) {
            pop();
        }
        return true;
    }

    bool visitSelection(TVisit visit, TIntermSelection* n) override {
        return visitNode(
                visit, "If", "if", typeStr(n->getType()), n->getLoc().line
        );
    }

    bool visitSwitch(TVisit visit, TIntermSwitch* n) override {
        return visitNode(
                visit, "Switch", "switch", "", n->getLoc().line
        );
    }

    bool visitLoop(TVisit visit, TIntermLoop* n) override {
        return visitNode(
                visit, "Loop", n->testFirst() ? "while" : "do-while", "", n->getLoc().line
        );
    }

    bool visitBranch(TVisit visit, TIntermBranch* n) override {
        return visitNode(
                visit, "Branch", opStr(n->getFlowOp()), "", n->getLoc().line
        );
    }

*/

};

#endif //PYGLSLANG_TRAVERSER_H

#ifndef PYGLSLANG_TRAVERSER_H
#define PYGLSLANG_TRAVERSER_H

#include <string>
#include <vector>
#include <memory>
#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>


struct ASTNode {
    std::string kind;
    std::string name;
    std::string type;
    int line = 0;
    std::vector<std::shared_ptr<ASTNode>> children;
};

static std::string opStr(glslang::TOperator op) {
    switch (op) {
        case glslang::EOpAssign: return "=";
        case glslang::EOpAddAssign: return "+=";
        case glslang::EOpSubAssign: return "-=";
        case glslang::EOpMulAssign: return "*=";
        case glslang::EOpDivAssign: return "/=";
        case glslang::EOpAdd: return "+";
        case glslang::EOpSub: return "-";
        case glslang::EOpMul: return "*";
        case glslang::EOpDiv: return "/";
        case glslang::EOpMod: return "%";
        case glslang::EOpNegative: return "-";
        case glslang::EOpEqual: return "==";
        case glslang::EOpNotEqual: return "!=";
        case glslang::EOpLessThan: return "<";
        case glslang::EOpGreaterThan: return ">";
        case glslang::EOpLessThanEqual: return "<=";
        case glslang::EOpGreaterThanEqual: return ">=";
        case glslang::EOpLogicalAnd: return "&&";
        case glslang::EOpLogicalOr: return "||";
        case glslang::EOpLogicalNot: return "!";
        case glslang::EOpFunctionCall: return "call";
        case glslang::EOpFunction: return "function";
        case glslang::EOpParameters: return "params";
        case glslang::EOpLinkerObjects: return "linker-objects";
        case glslang::EOpSequence: return "sequence";
        case glslang::EOpScope: return "scope";
        case glslang::EOpVectorSwizzle: return "swizzle";
        case glslang::EOpIndexDirect: return "index";
        case glslang::EOpIndexIndirect: return "index[]";
        case glslang::EOpIndexDirectStruct: return "field";
        case glslang::EOpDot: return "dot";
        case glslang::EOpCross: return "cross";
        case glslang::EOpNormalize: return "normalize";
        case glslang::EOpLength: return "length";
        case glslang::EOpDistance: return "distance";
        case glslang::EOpReflect: return "reflect";
        case glslang::EOpRefract: return "refract";
        case glslang::EOpMix: return "mix";
        case glslang::EOpClamp: return "clamp";
        case glslang::EOpSmoothStep: return "smoothstep";
        case glslang::EOpStep: return "step";
        case glslang::EOpMin: return "min";
        case glslang::EOpMax: return "max";
        case glslang::EOpPow: return "pow";
        case glslang::EOpExp: return "exp";
        case glslang::EOpExp2: return "exp2";
        case glslang::EOpLog: return "log";
        case glslang::EOpLog2: return "log2";
        case glslang::EOpSqrt: return "sqrt";
        case glslang::EOpInverseSqrt: return "inversesqrt";
        case glslang::EOpAbs: return "abs";
        case glslang::EOpSign: return "sign";
        case glslang::EOpFloor: return "floor";
        case glslang::EOpCeil: return "ceil";
        case glslang::EOpFract: return "fract";
        case glslang::EOpSin: return "sin";
        case glslang::EOpCos: return "cos";
        case glslang::EOpTan: return "tan";
        case glslang::EOpAsin: return "asin";
        case glslang::EOpAcos: return "acos";
        case glslang::EOpAtan: return "atan";
        case glslang::EOpTexture: return "texture";
        case glslang::EOpTextureProj: return "textureProj";
        case glslang::EOpTextureLod: return "textureLod";
        case glslang::EOpTextureFetch: return "texelFetch";
        case glslang::EOpReturn: return "return";
        case glslang::EOpBreak: return "break";
        case glslang::EOpContinue: return "continue";
        case glslang::EOpTerminateInvocation: return "discard";
        default: return "op(" + std::to_string((int)op) + ")";
    }
}

struct Traverser : public glslang::TIntermTraverser {
    std::shared_ptr<ASTNode> root;
    std::vector<ASTNode*> stack;

    Traverser()
            : glslang::TIntermTraverser(true, false, true)
            , root(std::make_shared<ASTNode>()) {
        root->kind = "Root";
        stack.push_back(root.get());
    }

    ASTNode* top() {
        return stack.back();
    }

    ASTNode* push(const std::basic_string<char> &kind,
                  const std::basic_string<char> &name,
                  const std::basic_string<char> &type,
                  int line) {
        auto node = std::make_shared<ASTNode>();
        node->kind = kind;
        node->name = name;
        node->type = type;
        node->line = line;
        top()->children.push_back(node);
        return node.get();
    }

    ASTNode* push(const std::basic_string<char> &kind,
                  const std::basic_string<char> &name,
                  const glslang::TIntermTyped *n) {
        return push(kind,
                    name,
                    typeStr(n->getType()),
                    n->getLoc().line);
    }

    ASTNode* push(const std::basic_string<char> &kind,
                  const glslang::TIntermOperator *n) {
        return push(kind, opStr(n->getOp()), n);
    }

    void to_stack(ASTNode* node) {
        stack.push_back(node);
    }

    void pop() {
        stack.pop_back();
    }

    static std::basic_string<char> typeStr(const glslang::TType& t) {
        return t.getCompleteString().c_str();
    }

    template<typename T>
    bool visitNode(glslang::TVisit visit, const std::string& kind, T* n) {
        if (visit == glslang::EvPreVisit) {
            to_stack(push(kind, n));
        }
        else if (visit == glslang::EvPostVisit) {
            pop();
        }
        return true;
    }

    bool visitNode(glslang::TVisit visit,
                   const std::string& kind,
                   const std::string& name,
                   const std::string& type,
                   int line) {
        if (visit == glslang::EvPreVisit) {
            to_stack(push(kind, name, type, line));
        }
        else if (visit == glslang::EvPostVisit) {
            pop();
        }
        return true;
    }

    void visitSymbol(glslang::TIntermSymbol* n) override {
        push("Symbol", n->getName().c_str(), n);
    }

    void visitConstantUnion(glslang::TIntermConstantUnion* n) override {
        const auto& arr = n->getConstArray();
        auto u = arr[0];
        std::string name;
        switch (n->getType().getBasicType()) {
            case glslang::EbtFloat:
                name = std::to_string(u.getDConst());
                break;
            case glslang::EbtInt:
                name = std::to_string(u.getIConst());
                break;
            case glslang::EbtUint:
                name = std::to_string(u.getUConst());
                break;
            case glslang::EbtBool:
                name = u.getBConst() ? "true" : "false";
                break;
            default:
                name = "?";
                break;
        }
        push("Constant", name, n);
    }

    bool visitBinary(glslang::TVisit visit, glslang::TIntermBinary* n) override {
        return visitNode(visit, "Binary", n);
    }

    bool visitUnary(glslang::TVisit visit, glslang::TIntermUnary* n) override {
        return visitNode(visit, "Unary", n);
    }

    bool visitAggregate(glslang::TVisit visit, glslang::TIntermAggregate* n) override {
        if (visit == glslang::EvPreVisit) {
            std::string name(n->getName());
            if (name.empty()) {
                name = opStr(n->getOp());
            }
            to_stack(push("Aggregate", name, n));
        }
        else if (visit == glslang::EvPostVisit) {
            pop();
        }
        return true;
    }

    bool visitSelection(glslang::TVisit visit, glslang::TIntermSelection* n) override {
        return visitNode(visit, "Selection", "if", typeStr(n->getType()), n->getLoc().line);
    }

    bool visitSwitch(glslang::TVisit visit, glslang::TIntermSwitch* n) override {
        return visitNode(visit, "Switch", "switch", "", n->getLoc().line);
    }

    bool visitLoop(glslang::TVisit visit, glslang::TIntermLoop* n) override {
        return visitNode(visit, "Loop", n->testFirst() ? "while" : "do-while", "", n->getLoc().line);
    }

    bool visitBranch(glslang::TVisit visit, glslang::TIntermBranch* n) override {
        return visitNode(visit, "Branch", opStr(n->getFlowOp()), "", n->getLoc().line);
    }
};

#endif //PYGLSLANG_TRAVERSER_H

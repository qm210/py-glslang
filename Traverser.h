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
            , root(std::make_shared<ASTNode>())
    {
        root->kind = "Root";
        stack.push_back(root.get());
    }

    ASTNode* top() { return stack.back(); }

    ASTNode* push(const std::basic_string<char> & kind,
                  const std::basic_string<char> & name,
                  const std::basic_string<char> & type,
                  int line)
    {
        auto node = std::make_shared<ASTNode>();
        node->kind = kind;
        node->name = name;
        node->type = type;
        node->line = line;
        top()->children.push_back(node);
        stack.push_back(node.get());
        return node.get();
    }

    void pop() { stack.pop_back(); }

    static std::basic_string<char> typeStr(const glslang::TType& t) {
        return t.getCompleteString().c_str();
    }

    void visitSymbol(glslang::TIntermSymbol* n) override {
        auto node = std::make_shared<ASTNode>();
        node->kind = "Symbol";
        node->name = n->getName().c_str();
        node->type = typeStr(n->getType());
        node->line = n->getLoc().line;
        top()->children.push_back(node);
    }

    void visitConstantUnion(glslang::TIntermConstantUnion* n) override {
        auto node = std::make_shared<ASTNode>();
        node->kind = "Constant";
        node->type = typeStr(n->getType());
        node->line = n->getLoc().line;
        const auto& arr = n->getConstArray();
        switch (n->getType().getBasicType()) {
            case glslang::EbtFloat:  node->name = std::to_string(arr[0].getDConst()); break;
            case glslang::EbtInt:    node->name = std::to_string(arr[0].getIConst()); break;
            case glslang::EbtUint:   node->name = std::to_string(arr[0].getUConst()); break;
            case glslang::EbtBool:   node->name = arr[0].getBConst() ? "true" : "false"; break;
            default:                 node->name = "?"; break;
        }
        top()->children.push_back(node);
    }

    bool visitBinary(glslang::TVisit visit, glslang::TIntermBinary* n) override {
        if (visit == glslang::EvPreVisit)
            push("Binary", opStr(n->getOp()), typeStr(n->getType()), n->getLoc().line);
        else if (visit == glslang::EvPostVisit)
            pop();
        return true;
    }

    bool visitUnary(glslang::TVisit visit, glslang::TIntermUnary* n) override {
        if (visit == glslang::EvPreVisit)
            push("Unary", opStr(n->getOp()), typeStr(n->getType()), n->getLoc().line);
        else if (visit == glslang::EvPostVisit)
            pop();
        return true;
    }

    bool visitAggregate(glslang::TVisit visit, glslang::TIntermAggregate* n) override {
        if (visit == glslang::EvPreVisit) {
            std::basic_string<char> name = n->getName().c_str();
            if (name.empty()) name = opStr(n->getOp());
            push("Aggregate", name, typeStr(n->getType()), n->getLoc().line);
        } else if (visit == glslang::EvPostVisit) {
            pop();
        }
        return true;
    }

    bool visitSelection(glslang::TVisit visit, glslang::TIntermSelection* n) override {
        if (visit == glslang::EvPreVisit)
            push("Selection", "if", typeStr(n->getType()), n->getLoc().line);
        else if (visit == glslang::EvPostVisit)
            pop();
        return true;
    }

    bool visitSwitch(glslang::TVisit visit, glslang::TIntermSwitch* n) override {
        if (visit == glslang::EvPreVisit)
            push("Switch", "switch", "", n->getLoc().line);
        else if (visit == glslang::EvPostVisit)
            pop();
        return true;
    }

    bool visitLoop(glslang::TVisit visit, glslang::TIntermLoop* n) override {
        if (visit == glslang::EvPreVisit)
            push("Loop", n->testFirst() ? "while" : "do-while", "", n->getLoc().line);
        else if (visit == glslang::EvPostVisit)
            pop();
        return true;
    }

    bool visitBranch(glslang::TVisit visit, glslang::TIntermBranch* n) override {
        if (visit == glslang::EvPreVisit)
            push("Branch", opStr(n->getFlowOp()), "", n->getLoc().line);
        else if (visit == glslang::EvPostVisit)
            pop();
        return true;
    }
};

#endif //PYGLSLANG_TRAVERSER_H

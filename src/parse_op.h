#ifndef PYGLSL_PARSE_OP_H
#define PYGLSL_PARSE_OP_H

#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <string>

using namespace glslang;

static const char* builtinName(TOperator op) {
    switch (op) {
        case EOpMod: return "mod";
        case EOpRadians: return "radians";
        case EOpDegrees: return "degrees";
        case EOpSin: return "sin";
        case EOpCos: return "cos";
        case EOpTan: return "tan";
        case EOpAsin: return "asin";
        case EOpAcos: return "acos";
        case EOpAtan: return "atan";
        case EOpSinh: return "sinh";
        case EOpCosh: return "cosh";
        case EOpTanh: return "tanh";
        case EOpAsinh: return "asinh";
        case EOpAcosh: return "acosh";
        case EOpAtanh: return "atanh";
        case EOpPow: return "pow";
        case EOpExp: return "exp";
        case EOpLog: return "log";
        case EOpExp2: return "exp2";
        case EOpLog2: return "log2";
        case EOpSqrt: return "sqrt";
        case EOpInverseSqrt: return "inversesqrt";
        case EOpAbs: return "abs";
        case EOpSign: return "sign";
        case EOpFloor: return "floor";
        case EOpTrunc: return "trunc";
        case EOpRound: return "round";
        case EOpRoundEven: return "roundEven";
        case EOpCeil: return "ceil";
        case EOpFract: return "fract";
        case EOpModf: return "modf";
        case EOpMin: return "min";
        case EOpMax: return "max";
        case EOpClamp: return "clamp";
        case EOpMix: return "mix";
        case EOpStep: return "step";
        case EOpSmoothStep: return "smoothstep";
        case EOpIsNan: return "isnan";
        case EOpIsInf: return "isinf";
        case EOpFma: return "fma";
        case EOpFrexp: return "frexp";
        case EOpLdexp: return "ldexp";
/* // TODO check all these...
    EOpFloatBitsToInt,
    EOpFloatBitsToUint,
    EOpIntBitsToFloat,
    EOpUintBitsToFloat,
    EOpDoubleBitsToInt64,
    EOpDoubleBitsToUint64,
    EOpInt64BitsToDouble,
    EOpUint64BitsToDouble,
    EOpFloat16BitsToInt16,
    EOpFloat16BitsToUint16,
    EOpInt16BitsToFloat16,
    EOpUint16BitsToFloat16,
    EOpPackSnorm2x16,
    EOpUnpackSnorm2x16,
    EOpPackUnorm2x16,
    EOpUnpackUnorm2x16,
    EOpPackSnorm4x8,
    EOpUnpackSnorm4x8,
    EOpPackUnorm4x8,
    EOpUnpackUnorm4x8,
    EOpPackHalf2x16,
    EOpUnpackHalf2x16,
    EOpPackDouble2x32,
    EOpUnpackDouble2x32,
    EOpPackInt2x32,
    EOpUnpackInt2x32,
    EOpPackUint2x32,
    EOpUnpackUint2x32,
    EOpPackFloat2x16,
    EOpUnpackFloat2x16,
    EOpPackInt2x16,
    EOpUnpackInt2x16,
    EOpPackUint2x16,
    EOpUnpackUint2x16,
    EOpPackInt4x16,
    EOpUnpackInt4x16,
    EOpPackUint4x16,
    EOpUnpackUint4x16,
    EOpPack16,
    EOpPack32,
    EOpPack64,
    EOpUnpack32,
    EOpUnpack16,
    EOpUnpack8,

    EOpLength,
    EOpDistance,
    EOpDot,
    EOpCross,
    EOpNormalize,
    EOpFaceForward,
    EOpReflect,
    EOpRefract,

    EOpMin3,
    EOpMax3,
    EOpMid3,

    EOpDPdx,            // Fragment only
    EOpDPdy,            // Fragment only
    EOpFwidth,          // Fragment only
    EOpDPdxFine,        // Fragment only
    EOpDPdyFine,        // Fragment only
    EOpFwidthFine,      // Fragment only
    EOpDPdxCoarse,      // Fragment only
    EOpDPdyCoarse,      // Fragment only
    EOpFwidthCoarse,    // Fragment only

    EOpInterpolateAtCentroid, // Fragment only
    EOpInterpolateAtSample,   // Fragment only
    EOpInterpolateAtOffset,   // Fragment only
    EOpInterpolateAtVertex,

    EOpMatrixTimesMatrix,
    EOpOuterProduct,
    EOpDeterminant,
    EOpMatrixInverse,
    EOpTranspose,

    EOpFtransform,

    EOpNoise,

 */
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
        case EOpAny: return "any";
        case EOpAll: return "all";
        default: return nullptr;
    }
}

static std::string opStr(TOperator op) {
    switch (op) {
        case EOpNegative: return "-";
        case EOpLogicalNot:
        case EOpVectorLogicalNot:
            return "!";
        case EOpBitwiseNot: return "~";
        case EOpPreIncrement:
        case EOpPostIncrement:
            return "++";
        case EOpPostDecrement:
        case EOpPreDecrement:
            return "--";
        case EOpAdd: return "+";
        case EOpSub: return "-";
        case EOpMul: return "*";
        case EOpDiv: return "/";
        case EOpMod: return "%";
        case EOpRightShift: return ">>";
        case EOpLeftShift: return "<<";
        case EOpAnd: return "&";
        case EOpInclusiveOr: return "|";
        case EOpExclusiveOr: return "^";
        case EOpEqual:
        case EOpVectorEqual:
            return "==";
        case EOpNotEqual:
        case EOpVectorNotEqual:
            return "!=";
        case EOpLessThan: return "<";
        case EOpGreaterThan: return ">";
        case EOpLessThanEqual: return "<=";
        case EOpGreaterThanEqual: return ">=";
        case EOpComma: return ",";
        case EOpVectorTimesMatrix:
        case EOpMatrixTimesVector:
        case EOpVectorTimesScalar:
        case EOpMatrixTimesScalar:
            return "*";
        case EOpLogicalAnd: return "&&";
        case EOpLogicalOr: return "||";
        case EOpLogicalXor: return "^^";
        case EOpAssign:
        case EOpVectorTimesMatrixAssign:
        case EOpVectorTimesScalarAssign:
        case EOpMatrixTimesScalarAssign:
        case EOpMatrixTimesMatrixAssign:
            return "=";
        case EOpAddAssign: return "+=";
        case EOpSubAssign: return "-=";
        case EOpMulAssign: return "*=";
        case EOpDivAssign: return "/=";
        case EOpModAssign: return "%=";
        case EOpAndAssign: return "&&=";
        case EOpInclusiveOrAssign: return "||=";
        case EOpExclusiveOrAssign: return "^^=";
        case EOpLeftShiftAssign: return "<<=";
        case EOpRightShiftAssign: return ">>=";

        case EOpVectorSwizzle:
            return ".";
        case EOpIndexDirect:
        case EOpIndexIndirect:
            return "[]";
        case EOpIndexDirectStruct:
            return ".";
        default: {
            if (const char* builtin = builtinName(op)) {
                return builtin;
            }
            // Missing OP -- WIP TODO make sure these are complete!
            return "[OP" + std::to_string((int)op) + "]";
        }
    }
}

#endif //PYGLSL_PARSE_OP_H

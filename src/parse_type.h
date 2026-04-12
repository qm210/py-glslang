#ifndef PYGLSLANG_PARSE_TYPE_H
#define PYGLSLANG_PARSE_TYPE_H

#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <string>

using namespace glslang;

static std::string precisionQualifier(const TType& t) {
    switch (t.getQualifier().precision) {
        case EpqLow: return "lowp";
        case EpqMedium: return "mediump";
        case EpqHigh: return "highp";
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
    if (!s.empty()) {
        s += " ";
    }
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

#endif //PYGLSLANG_PARSE_TYPE_H

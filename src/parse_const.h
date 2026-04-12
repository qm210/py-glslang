#ifndef PYGLSLANG_PARSE_CONST_H
#define PYGLSLANG_PARSE_CONST_H

#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <sstream>
#include <vector>
#include <string>


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
                return "???";
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

#endif //PYGLSLANG_PARSE_CONST_H

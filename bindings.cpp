#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <string>
#include <vector>

namespace py = pybind11;

void initialize() {
    glslang::InitializeProcess();
}

void finalize() {
    glslang::FinalizeProcess();
}

enum Stage {
    STAGE_VERT = 0,
    STAGE_TESC = 1,
    STAGE_TESE = 2,
    STAGE_GEOM = 3,
    STAGE_FRAG = 4,
    STAGE_COMP = 5
};

struct Parsed {
    bool ok;
    std::string info;
    std::string debug;
    std::vector<uint32_t> spirv;
};

Parsed parse_glsl(const std::string& source, Stage stage_enum) {
    auto stage = static_cast<EShLanguage>(stage_enum);
    glslang::TShader shader(stage);
    const char* src = source.c_str();
    shader.setStrings(&src, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, stage,
                       glslang::EShClientOpenGL, 450);
    shader.setEnvClient(glslang::EShClientOpenGL,
                        glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EShTargetSpv,
                        glslang::EShTargetSpv_1_0);

    Parsed result;
    result.ok = shader.parse(
            GetDefaultResources(), 450, false, EShMsgDefault
    );
    result.info  = shader.getInfoLog();
    result.debug = shader.getInfoDebugLog();
    if (!result.ok) {
        return result;
    }

    glslang::TProgram program;
    program.addShader(&shader);
    result.ok = program.link(EShMsgDefault);
    result.info += program.getInfoLog();
    if (!result.ok) {
        return result;
    }

    glslang::TIntermediate* intermediate = program.getIntermediate(stage);
    glslang::GlslangToSpv(*intermediate, result.spirv);
    return result;
}

PYBIND11_MODULE(pyglslang, m) {
    m.doc() = "Python bindings for Khronos Group GLSL parser";

    py::enum_<Stage>(m, "Stage")
            .value("VERT", STAGE_VERT)
            .value("TESC", STAGE_TESC)
            .value("TESE", STAGE_TESE)
            .value("GEOM", STAGE_GEOM)
            .value("FRAG", STAGE_FRAG)
            .value("COMP", STAGE_COMP)
            .export_values();
    py::class_<Parsed>(m, "Parsed")
        .def_readonly("ok", &Parsed::ok)
        .def_readonly("info", &Parsed::info)
        .def_readonly("debug", &Parsed::debug)
        .def_readonly("spirv", &Parsed::spirv);

    m.def("initialize", &initialize);
    m.def("finalize", &finalize);
    m.def("parse_glsl", &parse_glsl,
          py::arg("source"),
          py::arg("stage") = STAGE_FRAG);
}

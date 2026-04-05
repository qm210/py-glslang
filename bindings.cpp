#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <string>
#include <vector>

namespace py = pybind11;

void initialize() { glslang::InitializeProcess(); }
void finalize()   { glslang::FinalizeProcess(); }

struct ParseResult {
    bool success;
    std::string info_log;
    std::string debug_log;
    std::vector<uint32_t> spirv;
};

ParseResult parse_glsl(const std::string& source, int stage_int) {
    // stage_int: 0=Vert, 1=Tesc, 2=Tese, 3=Geom, 4=Frag, 5=Comp
    auto stage = static_cast<EShLanguage>(stage_int);

    glslang::TShader shader(stage);
    const char* src = source.c_str();
    shader.setStrings(&src, 1);

    shader.setEnvInput(glslang::EShSourceGlsl, stage,
                       glslang::EShClientOpenGL, 450);
    shader.setEnvClient(glslang::EShClientOpenGL,
                        glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EShTargetSpv,
                        glslang::EShTargetSpv_1_0);

    ParseResult result;
    bool ok = shader.parse(GetDefaultResources(), 450, false, EShMsgDefault);
    result.info_log  = shader.getInfoLog();
    result.debug_log = shader.getInfoDebugLog();

    if (!ok) {
        result.success = false;
        return result;
    }

    glslang::TProgram program;
    program.addShader(&shader);
    ok = program.link(EShMsgDefault);
    result.info_log += program.getInfoLog();

    if (!ok) {
        result.success = false;
        return result;
    }

    glslang::TIntermediate* intermediate = program.getIntermediate(stage);
    glslang::GlslangToSpv(*intermediate, result.spirv);
    result.success = true;
    return result;
}

PYBIND11_MODULE(glslang_py, m) {
    m.doc() = "Python bindings for Khronos Group GLSL parser";
    m.def("initialize", &initialize);
    m.def("finalize",   &finalize);
    m.def("parse_glsl", &parse_glsl,
          py::arg("source"), py::arg("stage") = 4);

    py::class_<ParseResult>(m, "ParseResult")
        .def_readonly("success",   &ParseResult::success)
        .def_readonly("info_log",  &ParseResult::info_log)
        .def_readonly("debug_log", &ParseResult::debug_log)
        .def_readonly("spirv",     &ParseResult::spirv);
}

#include "Parsed.h"
#include "Traverser.h"

Parsed parse(const std::string& source, Stage stage_enum) {
    auto stage = static_cast<EShLanguage>(stage_enum);
    glslang::TShader shader(stage);
    const char* src = source.c_str();
    const char* name = "shader";
    shader.setStringsWithLengthsAndNames(&src, nullptr, &name, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, stage,
                       glslang::EShClientOpenGL, 450);
    shader.setEnvClient(glslang::EShClientOpenGL,
                        glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EShTargetSpv,
                        glslang::EShTargetSpv_1_0);

    Parsed result;
    result.ok = shader.parse(
            GetDefaultResources(),
            450,
            false,
            static_cast<EShMessages>(EShMsgDefault | EShMsgDebugInfo)
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

    Traverser traverser;
    intermediate->getTreeRoot()->traverse(&traverser);
    result.ast = traverser.root;

    return result;
}

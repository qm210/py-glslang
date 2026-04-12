#include "module.h"
#include "Traverser.h"

Parsed parse_when_initialized(const std::string& source, Stage stage_enum) {
    auto stage = static_cast<EShLanguage>(stage_enum);
    glslang::TShader shader(stage);
    const char* src = source.c_str();
    // name arbitrary, as we only have one shader / stage here
    const char* name = "shader";
    shader.setStringsWithLengthsAndNames(&src, nullptr, &name, 1);
    shader.setEnvInput(glslang::EShSourceGlsl,
                       stage,
                       glslang::EShClientOpenGL,
                       450);
    shader.setEnvClient(glslang::EShClientOpenGL,
                        glslang::EShTargetOpenGL_450);
    Parsed result;
    result.ok = shader.parse(
            GetDefaultResources(),
            450,
            false,
            static_cast<EShMessages>(EShMsgDefault
                                   | EShMsgDebugInfo)
    );
    result.info  = shader.getInfoLog();
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
    Traverser traverser(*intermediate);
    intermediate->getTreeRoot()->traverse(&traverser);
    result.node = traverser.build();
    return result;
}

Parsed parse(const std::string& source, Stage stage) {
    glslang::InitializeProcess();
    Parsed parsed = parse_when_initialized(source, stage);
    glslang::FinalizeProcess();
    return parsed;
}

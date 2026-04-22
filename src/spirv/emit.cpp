#include "module.h"

#include <glslang/Public/ShaderLang.h>
#include <spirv_glsl.hpp>
#include "FloatFormatter.h"

std::string emitFromSpirv(const Spirv& spirv) {
    spirv_cross::CompilerGLSL compiler(spirv);
    auto resources = compiler.get_shader_resources();

    spirv_cross::CompilerGLSL::Options opts;
    opts.version = 450;
    opts.es = false;
    opts.vulkan_semantics = false;
    opts.vertex.fixup_clipspace = true;
    opts.vertex.flip_vert_y = false;
    compiler.set_common_options(opts);

    CustomFloatFormatter formatter;
    compiler.set_float_formatter(&formatter);

    return compiler.compile();
}

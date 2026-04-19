#include "module.h"

#include <glslang/Public/ShaderLang.h>
#include <spirv_glsl.hpp>

std::string emitFromSpirv(const Spirv& spirv) {
    spirv_cross::CompilerGLSL compiler(spirv);
    auto resources = compiler.get_shader_resources();
    for (auto& img : resources.sampled_images)
    {
        if (compiler.has_decoration(img.id, spv::DecorationDescriptorSet))
            compiler.unset_decoration(img.id, spv::DecorationDescriptorSet);
    }

    spirv_cross::CompilerGLSL::Options opts;
    opts.version = 450;
    opts.es = false;
    opts.vulkan_semantics = false;
    compiler.set_common_options(opts);

    return compiler.compile();
}
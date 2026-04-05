import sys
sys.path.insert(0, "build/")   # or install with pip
import pyglslang

pyglslang.initialize()

frag_src = """
    #version 450
    layout(location = 0) in vec2 vUV;
    layout(location = 0) out vec4 fragColor;
    uniform sampler2D uTex;
    void main() {
        fragColor = texture(uTex, vUV);
    }
"""

# 4 = fragment
result = pyglslang.parse_glsl(frag_src, stage=4)

if result.success:
    print("Parse OK")
    print(f"SPIR-V word count: {len(result.spirv)}")
else:
    print("Parse FAILED:")
    print(result.info_log)

pyglslang.finalize()

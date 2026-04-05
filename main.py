import sys
from argparse import ArgumentParser
from enum import Enum

sys.path.insert(0, "build/Release")
import pyglslang

class Stage(Enum):
    Vert = 0
    Tesc = 1
    Tese = 2
    Geom = 3
    Frag = 4
    Comp = 5

def run(shader_source: str):
    pyglslang.initialize()

    try:
        result = pyglslang.parse_glsl(
            shader_source,
            stage=pyglslang.Stage.FRAG
        )
    finally:
        pyglslang.finalize()

    if result.ok:
        print("Parse OK")
        print(f"SPIR-V word count: {len(result.spirv)}")
    else:
        print("Parse FAILED:")
        print(result.info)


def parse_cli():
    parser = ArgumentParser()
    parser.add_argument("shader",
                        nargs="?",
                        default="graphics.frag",
                        help="Path to shader file (default \"graphics.frag\")")
    args = parser.parse_args()
    with open(args.shader, "r", encoding="utf-8") as f:
        source = f.read()
    return args, source


if __name__ == "__main__":
    _args, shader = parse_cli()
    run(shader)

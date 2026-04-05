import sys
from argparse import ArgumentParser

sys.path.insert(0, "build")
import pyglslang


def print_node(node: pyglslang.ASTNode, level=0):
    indent = level * 2 * " "
    print(indent, node.kind, node.type, node.name)
    for child in node.children:
        print_node(child, level + 1)


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
        print("== AST ==")
        print_node(result.ast)
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

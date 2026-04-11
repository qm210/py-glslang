import sys
from argparse import ArgumentParser

sys.path.insert(0, "build")
import pyglslang as pyglsl


def write_node(node: pyglsl.Node, level=0, previous="") -> str:
    indent = level * 2 * " "
    result = f"{indent}{node.type} \"{node.name}\" ({node.kind})"
    if previous:
        result = f"{previous}\n{result}"
    for child in node.children:
        result = write_node(child, level + 1, result)
    return result


def render(root: pyglsl.Node):
    return "TODO"


def run(args):
    with open(args.shader, "r", encoding="utf-8") as f:
        source = f.read()

    result = pyglsl.parse(source)
    if result.ok:
        print("Parse OK")
        print(f"SPIR-V word count: {len(result.spirv)}")
        ast = pyglsl.simplify(result.ast)
        code = pyglsl.emit(ast)
        ast_printed = write_node(ast)
        
        written = render(ast)
        if args.outfile:
            with open(args.outfile, "w", encoding="utf-8") as f:
                f.write(ast_printed)
        else:
            print("== AST ==")
            print(ast_printed)
    else:
        print("Parse FAILED:")
        print(result.info)


def parse_cli():
    parser = ArgumentParser()
    parser.add_argument("shader",
                        nargs="?",
                        default="graphics.frag",
                        help="Path to shader file (default \"graphics.frag\")")
    parser.add_argument("outfile",
                        nargs="?",
                        # default="ast.out",
                        help="Write the AST indented to some text file")
    return parser.parse_args()


if __name__ == "__main__":
    run(parse_cli())

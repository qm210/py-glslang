#include <glslang/Public/ShaderLang.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <string>
#include <vector>
#include <memory>

#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "module.h"

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

    py::class_<NodeSource>(m, "NodeSource")
            .def_readwrite("line", &NodeSource::line)
            .def_readwrite("column", &NodeSource::column)
            .def_readwrite("code", &NodeSource::code)
            .def("__repr__", [](const NodeSource& src) {
                return "NodeSource(" + std::to_string(src.line)
                    + ":" + std::to_string(src.column)
                    + ": \"" + src.code + "\")";
            });

    py::class_<Node, NodePtr>(m, "Node")
            .def_readonly("src", &Node::src)
            .def("__repr__", [](const Node& n) {
                auto s = "Node(" + n.kind();
                if (!n.label().empty()) {
                    s += " " + n.label();
                }
                return s + ", l." + std::to_string(n.src.line)
                    + ":" + std::to_string(n.src.column) + ")";
            })
            .def_property_readonly("kind", &Node::kind)
            .def_property_readonly("name", [](Node& n) -> std::string {
                if (auto *s = n.data_if<SymbolNode>())
                    return s->name;
                if (auto *s = n.data_if<DeclareNode>())
                    return s->name;
                if (auto *s = n.data_if<FunctionNode>())
                    return s->name;
                if (auto *s = n.data_if<CallNode>())
                    return s->functionName;
                return "";
            })
            .def_property_readonly("typeName", [](Node& n) -> std::string {
                if (auto *s = n.data_if<SymbolNode>())
                    return s->typeName;
                if (auto *s = n.data_if<DeclareNode>())
                    return s->typeName;
                if (auto *s = n.data_if<FunctionNode>())
                    return s->returnType;
                if (auto *s = n.data_if<ConstructNode>())
                    return s->typeName;
                return "";
            })
            .def_property_readonly("value", [](Node& n) -> py::object {
                if (auto *s = n.data_if<ConstantNode>())
                    return py::str(s->value);
                if (auto *s = n.data_if<BinaryNode>())
                    return py::str(s->op);
                if (auto *s = n.data_if<UnaryNode>())
                    return py::str(s->op);
                if (auto *s = n.data_if<UnaryNode>())
                    return py::cast(s->operand);
                if (auto *s = n.data_if<ReturnNode>())
                    return py::cast(s->value);
                if (auto *s = n.data_if<IfNode>())
                    return py::cast(s->condition);
                if (auto *s = n.data_if<LoopNode>())
                    return py::cast(s->condition);
                if (auto *s = n.data_if<SwitchNode>())
                    return py::cast(s->condition);
                if (auto *s = n.data_if<CaseNode>())
                    return py::cast(s->label);
                return py::none();
            })
            .def_property_readonly("children", [](Node& n) -> NodePtrs {
                if (auto *s = n.data_if<RootNode>())
                    return s->children;
                if (auto *s = n.data_if<SequenceNode>())
                    return s->statements;
                if (auto *s = n.data_if<CallNode>())
                    return s->args;
                if (auto *s = n.data_if<ConstructNode>())
                    return s->args;
                if (auto *s = n.data_if<FunctionNode>())
                    return s->body;
                if (auto *s = n.data_if<IfNode>())
                    return s->trueBranch;
                if (auto *s = n.data_if<SwitchNode>())
                    return s->cases;
                if (auto *s = n.data_if<CaseNode>())
                    return s->body;
                if (auto *s = n.data_if<LoopNode>())
                    return s->body;
                if (auto *s = n.data_if<BinaryNode>())
                    return NodePtrs{ s->lhs, s->rhs };
                if (auto *s = n.data_if<DeclareNode>())
                    return NodePtrs{ s->value };
                return {};
            })
            .def_property_readonly("childrenElse", [](Node& n) -> NodePtrs {
                if (auto *s = n.data_if<IfNode>())
                    return s->falseBranch;
                if (auto *s = n.data_if<RootNode>())
                    return s->globals;
                    // <-- TODO: should have its own thing...
                return {};
            });

    py::class_<Parsed>(m, "Parsed")
        .def_readonly("ok", &Parsed::ok)
        .def_readonly("info", &Parsed::info)
        .def_readonly("debug", &Parsed::debug)
        .def_readonly("ast", &Parsed::ast)
        .def_readonly("spirv", &Parsed::spirv);

    m.def(
            "parse",
            &parse,
            py::arg("source"),
            py::arg("stage") = STAGE_FRAG
    );
    m.def(
            "simplify",
            &simplify,
            py::arg("node")
    );
    m.def(
            "emit",
            [](const std::shared_ptr<Node>& node) {
                return emit(node);
            },
            py::arg("node")
    );
}

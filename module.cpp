#include <pybind11/pybind11.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <string>
#include <vector>
#include <memory>

#include "Node.h"
#include "enums.h"
#include "Parsed.h"

namespace py = pybind11;

Parsed parse(const std::string& source, enums stage_enum);
std::basic_string<char> render(const Node& node, int level = 0);

PYBIND11_MODULE(pyglslang, m) {
    m.doc() = "Python bindings for Khronos Group GLSL parser";

    py::enum_<enums>(m, "Stage")
            .value("VERT", STAGE_VERT)
            .value("TESC", STAGE_TESC)
            .value("TESE", STAGE_TESE)
            .value("GEOM", STAGE_GEOM)
            .value("FRAG", STAGE_FRAG)
            .value("COMP", STAGE_COMP)
            .export_values();
    py::class_<Node, std::shared_ptr<Node>>(m, "Node")
            .def_readonly("kind", &Node::kind)
            .def_readonly("name", &Node::name)
            .def_readonly("type", &Node::type)
            .def_readonly("line", &Node::line)
            .def_readonly("children", &Node::children)
            .def("__repr__", [](const Node& n) {
                return "<Node " + n.kind + " '" + n.name + "' : " + n.type + ">";
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
            [](const std::shared_ptr<Node>& node) {
                return simplify(*node);
            },
            py::arg("root")
    );
    m.def(
            "count_nodes",
            [](const std::shared_ptr<Node>& node) {
                return node->countTotal();
            },
            py::arg("root")
    );
    m.def(
            "render",
            [](const std::shared_ptr<Node>& node) {
                return render(*node);
            },
            py::arg("root")
    );
}

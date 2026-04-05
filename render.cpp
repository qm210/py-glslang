#include <sstream>
#include "Node.h"

std::string render(const Node& node, int depth = 0);
static inline std::string expr(Node node);

const int INDENT = 4;

static inline std::string indent(int level) {
    return std::string(level * INDENT, ' ');
}

static inline std::string children(const Node& node, int depth) {
    std::string out;
    for (const auto& child : node.children)
        out += render(*child, depth);
    return out;
}

static inline std::string call(const Node& node) {
    std::string out = node.name + "(";
    for (size_t i = 0; i < node.children.size(); ++i) {
        if (i) {
            out += ", ";
        }
        out += expr(node.child(i));
    }
    return out + ")";
}

static inline std::string expr(const Node node) {
    if (node.kind == "Symbol" || node.kind == "Constant") {
        return node.name;
    }
    else if (node.kind == "Call") {
        return call(node);
    } else if (node.kind == "Binary") {
        return "("
                + expr(node.child())
                + " " + node.name + " "
                + expr(node.child())
                + ")";
    } else if (node.kind == "Unary") {
        return node.name + expr(node.child());
    } else if (node.kind == "Member") {
        return expr(node.child()) + "." + node.name;
    } else if (node.kind == "Index") {
        return expr(node.child()) + "[" + expr(node.child(1)) + "]";
    } else {
        return node.name;
    }
}

std::string render(const Node& node, int depth) {
    std::ostringstream out;
    const auto ind = indent(depth);

    if (node.kind == "Root") {
        for (const auto& child : node.children)
            out << render(*child, 0) << "\n";
    }
    else if (node.kind == "Function") {
        out << node.type << " " << node.name << "(";
        if (!node.children.empty() && node.children[0]->kind == "Params") {
            const auto& params = node.children[0]->children;
            for (size_t i = 0; i < params.size(); ++i) {
                if (i) out << ", ";
                out << params[i]->type << " " << params[i]->name;
            }
            out << ") {\n";
            for (size_t i = 1; i < node.children.size(); ++i)
                out << render(*node.children[i], depth + 1);
        } else {
            out << ") {\n";
            out << children(node, depth + 1);
        }
        out << ind << "}\n";
    }
    else if (node.kind == "Block") {
        out << children(node, depth);
    }
    else if (node.kind == "If") {
        // children[0] = condition, [1] = then-block, [2] = else-block (optional)
        out << ind << "if (" << expr(*node.children[0]) << ") {\n";
        out << render(*node.children[1], depth + 1);
        out << ind << "}";
        if (node.children.size() > 2) {
            out << " else {\n";
            out << render(*node.children[2], depth + 1);
            out << ind << "}";
        }
        out << "\n";
    }
    else if (node.kind == "Return") {
        out << ind << "return";
        if (!node.children.empty())
            out << " " << expr(*node.children[0]);
        out << ";\n";
    }
    else if (node.kind == "Assign") {
        out << ind << expr(*node.children[0])
            << " = " << expr(*node.children[1]) << ";\n";
    }
    else if (node.kind == "Declare") {
        out << ind << node.type << " " << node.name;
        if (!node.children.empty())
            out << " = " << expr(*node.children[0]);
        out << ";\n";
    }
    else if (node.kind == "Call") {
        out << ind << expr(node) << ";\n";
    }
    else if (node.kind == "Symbol" || node.kind == "Constant") {
        out << ind << expr(node) << ";\n";
    }
    return out.str();
}
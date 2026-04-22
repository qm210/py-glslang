#ifndef PYGLSL_TRAVERSER_H
#define PYGLSL_TRAVERSER_H

#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

#include "Node.h"
#include "NodeSource.h"
#include "parse_op.h"
#include "parse_type.h"
#include "parse_const.h"
#include "TraverseLog.h"

using namespace glslang;


struct Traverser : public TIntermTraverser {
private:
    std::vector<NodePtrs> stack;
    const TIntermediate& intermediate;
    TraverseLogs logs;

    NodePtrs& top() {
        return stack.back();
    }

    NodePtrs pop() {
        auto vec = std::move(top());
        stack.pop_back();
        return vec;
    }

    void pushStack() {
        stack.emplace_back();
    }

    template <typename T, typename... Args>
    void addNode(TIntermNode* n, Args&&... args) {
        NodePtr node = Node::make<T>(
                src(n), std::forward<Args>(args)...
        );
        top().push_back(node);
    }

    NodeSource src(TIntermNode* n) {
        return NodeSource::find(n, intermediate);
    }

    NodePtrs traverseChildren(TIntermNode* n) {
        if (!n) {
            return NodePtrs{};
        }
        pushStack();
        n->traverse(this);
        return pop();
    }

    void logVisit(std::string_view kind, TIntermNode *n, TVisit visit = EvPreVisit) {
        auto source = src(n);
        logs.push_back(TraverseLog{
                kind, "", visit, "",
                source.to_str(), stack.size(), top().size()
        });
    }

    void logVisit(std::string_view kind, TIntermTyped *n, TVisit visit = EvPreVisit) {
        auto source = src(n);
        logs.push_back(TraverseLog{
                kind, typeStr(n->getType()), visit,
                std::string(n->getType().getCompleteString(true)),
                source.to_str(), stack.size(), top().size()
        });
    }

    void logVisit(std::string_view kind, TIntermTyped *n, std::string name) {
        auto source = src(n);
        logs.push_back(TraverseLog{
                kind, typeStr(n->getType()) + " " + name, EvPreVisit,
                std::string(n->getType().getCompleteString(true)),
                source.to_str(), stack.size(), top().size()
        });
    }

    void logVisit(std::string_view kind, TIntermOperator *n, TVisit visit, std::string name = "") {
        auto source = src(n);
        logs.push_back(TraverseLog{
                kind, opStr(n->getOp()) + " " + name, visit,
                std::string(n->getType().getCompleteString(true)),
                source.to_str(), stack.size(), top().size()
        });
    }

public:
    explicit Traverser(const TIntermediate& intermediate)
        : TIntermTraverser(true, false, true, false, true)
        , intermediate(intermediate)
    {
        pushStack();
    }

    NodePtr build();

    [[nodiscard]]
    const TraverseLogs& logged() const { return logs; }

    /*
     * Note: the bool return value means (only for EvPreVisit)
     * true: automatically traverse the children afterwards
     * false: no automatic traversal, need to do manually then
     *        -> will also suppress EvPostVisit!
     */

    void visitSymbol(TIntermSymbol* n) override;
    void visitConstantUnion(TIntermConstantUnion* n) override;
    bool visitBinary(TVisit visit, TIntermBinary* n) override;
    bool visitUnary(TVisit visit, TIntermUnary* n) override;
    bool visitAggregate(TVisit visit, TIntermAggregate* n) override;
    bool visitSelection(TVisit visit, TIntermSelection* n) override;
    bool visitSwitch(TVisit visit, TIntermSwitch* n) override;
    bool visitLoop(TVisit visit, TIntermLoop* n) override;
    bool visitBranch(TVisit visit, TIntermBranch* n) override;

};

#endif //PYGLSL_TRAVERSER_H

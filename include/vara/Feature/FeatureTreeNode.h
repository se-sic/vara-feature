#ifndef VARA_FEATURE_FEATURETREENODE_H
#define VARA_FEATURE_FEATURETREENODE_H

#include "vara/Feature/FeatureSourceRange.h"

#include "llvm/ADT/DirectedGraph.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#include <llvm/ADT/SetVector.h>
#include <set>
#include <stack>
#include <utility>
#include <variant>

using std::string;

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                          FeatureTreeNode Class
//===----------------------------------------------------------------------===//

class FeatureTreeNode : private llvm::DGNode<FeatureTreeNode, FeatureTreeNode> {
public:
  enum class NodeKind { NK_FEATURE, NK_RELATIONSHIP };

  using NodeSetType = typename llvm::SmallSet<FeatureTreeNode *, 3>;

  FeatureTreeNode(NodeKind Kind) : Kind(Kind){};
  FeatureTreeNode(const FeatureTreeNode &) = delete;
  FeatureTreeNode &operator=(const FeatureTreeNode &) = delete;
  virtual ~FeatureTreeNode() = default;

  [[nodiscard]] NodeKind getKind() const { return Kind; }

  //===--------------------------------------------------------------------===//
  // Parent

  [[nodiscard]] FeatureTreeNode *getParent() const { return Parent; }

  [[nodiscard]] bool isRoot() const { return Parent == nullptr; }

  bool hasEdgeFrom(FeatureTreeNode &N) const { return Parent == &N; }

  //===--------------------------------------------------------------------===//
  // Children

  [[nodiscard]] iterator begin() { return DGNode::begin(); }
  [[nodiscard]] const_iterator begin() const { return DGNode::begin(); }

  [[nodiscard]] iterator end() { return DGNode::end(); }
  [[nodiscard]] const_iterator end() const { return DGNode::end(); }

  [[nodiscard]] llvm::iterator_range<iterator> children() {
    return llvm::make_range(begin(), end());
  }
  [[nodiscard]] llvm::iterator_range<const_iterator> children() const {
    return llvm::make_range(begin(), end());
  }

  [[nodiscard]] bool hasEdgeTo(const FeatureTreeNode &N) const {
    return std::find(begin(), end(), &N) != end();
  }

protected:
  FeatureTreeNode(NodeKind Kind, FeatureTreeNode *Parent,
                  const NodeSetType &Children)
      : Kind(Kind), Parent(Parent) {
    this->Edges.insert(Children.begin(), Children.end());
  }

private:
  friend class FeatureModel;
  friend class FeatureModelBuilder;

  void setParent(FeatureTreeNode &Feature) { Parent = &Feature; }
  void setParent(FeatureTreeNode *Feature) { Parent = Feature; }

  bool addEdge(FeatureTreeNode *Feature) { return DGNode::addEdge(*Feature); }

  FeatureTreeNode *Parent{nullptr};
  NodeKind Kind;
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURETREENODE_H

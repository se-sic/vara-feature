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
  FeatureTreeNode(FeatureTreeNode &) = delete;
  FeatureTreeNode &operator=(FeatureTreeNode &) = delete;
  FeatureTreeNode(FeatureTreeNode &&) = delete;
  FeatureTreeNode &operator=(FeatureTreeNode &&) = delete;
  virtual ~FeatureTreeNode() = default;

  [[nodiscard]] NodeKind getKind() const { return Kind; }

  //===--------------------------------------------------------------------===//
  // Parent

  [[nodiscard]] bool isRoot() const { return Parent == nullptr; }

  /// Search parent of given type in tree structure.
  ///
  /// \return parent or nullptr if this is root or no parent exists
  [[nodiscard]] FeatureTreeNode *getParent() const { return Parent; }

  [[nodiscard]] bool hasEdgeFrom(FeatureTreeNode &N) const {
    return Parent == &N;
  }

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

  /// Search all children of given type in tree structure.
  template <typename T> llvm::SmallSet<T *, 3> getChildren() {
    llvm::SmallSet<T *, 3> FS;
    getChildrenImpl(this, &FS);
    return FS;
  }

protected:
  FeatureTreeNode(NodeKind Kind, FeatureTreeNode *Parent,
                  const llvm::SmallPtrSetImpl<FeatureTreeNode *> &Children)
      : Kind(Kind), Parent(Parent) {
    this->Edges.insert(Children.begin(), Children.end());
  }

  FeatureTreeNode(NodeKind Kind, FeatureTreeNode *Parent,
                  const std::vector<FeatureTreeNode *> &Children)
      : Kind(Kind), Parent(Parent) {
    this->Edges.insert(Children.begin(), Children.end());
  }

private:
  friend class FeatureModel;
  friend class FeatureModelBuilder;

  void setParent(FeatureTreeNode &Feature) { Parent = &Feature; }
  void setParent(FeatureTreeNode *Feature) { Parent = Feature; }

  bool addEdge(FeatureTreeNode *Feature) { return DGNode::addEdge(*Feature); }

  template <typename T>
  static void getChildrenImpl(FeatureTreeNode *N,
                              llvm::SmallPtrSetImpl<T *> *FS) {
    for (auto *C : N->children()) {
      if (auto *F = llvm::dyn_cast<T>(C); F) {
        FS->insert(F);
      } else {
        getChildrenImpl(C, FS);
      }
    }
  }

  const NodeKind Kind;
  FeatureTreeNode *Parent{nullptr};
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURETREENODE_H

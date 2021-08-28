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

namespace detail {
class FeatureModelModification;
} // namespace detail

//===----------------------------------------------------------------------===//
//                          FeatureTreeNode Class
//===----------------------------------------------------------------------===//

class FeatureTreeNode : private llvm::DGNode<FeatureTreeNode, FeatureTreeNode> {
  friend class FeatureModel;
  friend class FeatureModelBuilder;
  friend class detail::FeatureModelModification;

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

  /// Search first (possibly transitive) children of given type in tree
  ///  structure.
  ///
  /// \tparam[in] T Type of expected nodes.
  ///
  /// \param[in] Depth Maximal recursion depth to which nodes, which does not
  ///  match T are expanded to their children. A depth of one means, only
  ///  children of this node are considered and disables expansions.
  ///
  /// \return Set of children nodes.
  template <typename T = FeatureTreeNode>
  llvm::SmallSet<T *, 3> getChildren(int Depth = -1) {
    llvm::SmallSet<T *, 3> FS;
    getChildrenImpl(this, &FS, Depth);
    return FS;
  }

  /// Checks if a FeatureTreeNode has children or is a leave.
  /// \return true, if the FeatureTreeNode does not have children/is a leave.
  bool isLeave() { return this->children().empty(); }

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
  void setParent(FeatureTreeNode *Feature) { Parent = Feature; }

  bool addEdge(FeatureTreeNode *Feature) { return DGNode::addEdge(*Feature); }

  void removeEdge(FeatureTreeNode *Feature) {
    return DGNode::removeEdge(*Feature);
  }

  template <typename T>
  static void getChildrenImpl(FeatureTreeNode *N,
                              llvm::SmallPtrSetImpl<T *> *FS, int Depth) {
    if (Depth == 0) {
      return;
    }
    for (auto *C : N->children()) {
      if (auto *F = llvm::dyn_cast<T>(C); F) {
        FS->insert(F);
      } else {
        getChildrenImpl(C, FS, Depth - 1);
      }
    }
  }

  const NodeKind Kind;
  FeatureTreeNode *Parent{nullptr};
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURETREENODE_H

#ifndef VARA_FEATURE_NODE_H
#define VARA_FEATURE_NODE_H

#include "vara/Feature/FeatureSourceRange.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SetVector.h"
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
//                               Node Class
//===----------------------------------------------------------------------===//

class Node {
public:
  enum class NodeKind { NK_FEATURE, NK_GROUP };

  using NodeSetType = typename std::set<Node *>;
  using node_iterator = typename NodeSetType::iterator;
  using const_node_iterator = typename NodeSetType::const_iterator;

  Node(NodeKind Kind) : Kind(Kind){};
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;
  virtual ~Node() = default;

  [[nodiscard]] NodeKind getNodeKind() const { return Kind; }

  [[nodiscard]] bool isRoot() const { return Parent == nullptr; }

  [[nodiscard]] Node *getParent() const { return Parent; }
  bool isParent(Node *PosParent) const { return Parent == PosParent; }

  //===--------------------------------------------------------------------===//
  // Children

  node_iterator begin() { return Children.begin(); }
  node_iterator end() { return Children.end(); }
  [[nodiscard]] const_node_iterator begin() const { return Children.begin(); }
  [[nodiscard]] const_node_iterator end() const { return Children.end(); }
  llvm::iterator_range<node_iterator> children() {
    return llvm::make_range(begin(), end());
  }
  [[nodiscard]] llvm::iterator_range<const_node_iterator> children() const {
    return llvm::make_range(begin(), end());
  }
  bool isChild(Node *PosChild) const {
    return std::find(begin(), end(), PosChild) != end();
  }

protected:
  Node(NodeKind Kind, Node *Parent, NodeSetType Children)
      : Kind(Kind), Parent(Parent), Children(std::move(Children)) {}

private:
  friend class FeatureModel;
  friend class FeatureModelBuilder;

  void addChild(Node *Feature) { Children.insert(Feature); }

  void setParent(Node *Feature) { Parent = Feature; }

  Node *Parent{nullptr};
  NodeSetType Children;
  NodeKind Kind;
};
} // namespace vara::feature

#endif // VARA_FEATURE_NODE_H

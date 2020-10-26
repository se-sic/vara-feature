#ifndef VARA_FEATURE_RELATIONSHIP_H
#define VARA_FEATURE_RELATIONSHIP_H

#include "vara/Feature/FeatureTreeNode.h"

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
//                               Group Class
//===----------------------------------------------------------------------===//

class Relationship : public FeatureTreeNode {
public:
  enum class RelationshipKind { RK_ALTERNATIVE, RK_OR };

  Relationship(RelationshipKind Kind)
      : Kind(Kind), FeatureTreeNode(NodeKind::NK_GROUP) {}

  [[nodiscard]] RelationshipKind getRelationshipKind() const { return Kind; }

  static bool classof(const FeatureTreeNode *N) {
    return N->getNodeKind() == NodeKind::NK_GROUP;
  }

private:
  RelationshipKind Kind;
};
} // namespace vara::feature

#endif // VARA_FEATURE_RELATIONSHIP_H

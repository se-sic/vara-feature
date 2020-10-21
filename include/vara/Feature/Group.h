#ifndef VARA_FEATURE_GROUP_H
#define VARA_FEATURE_GROUP_H

#include "vara/Feature/Node.h"

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

class Group : public Node {
public:
  Group() : Node(NodeKind::NK_GROUP) {}

  static bool classof(const Node *N) {
    return N->getNodeKind() == NodeKind::NK_GROUP;
  }
};

class Alternative : public Group {};

class Or : public Group {};

} // namespace vara::feature

#endif // VARA_FEATURE_GROUP_H

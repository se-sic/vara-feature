#ifndef VARA_FEATURE_ERROR_H
#define VARA_FEATURE_ERROR_H

#include "vara/Utils/Result.h"
#include "vara/Utils/VariantUtil.h"

#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <iostream>
#include <type_traits>
#include <variant>

namespace vara {
namespace feature {

enum FTErrorCode {
  ABORTED,
  ALREADY_PRESENT,
  ERROR,
  INCONSISTENT,
  MISSING_FEATURE,
  MISSING_MODEL,
  MISSING_PARENT,
  NON_LEAF_NODE,
  CONSTRAINT_MALFORMED
};

} // namespace feature

template <>
class Error<vara::feature::FTErrorCode> {
public:
  Error(vara::feature::FTErrorCode E) : E(E) {}

  vara::feature::FTErrorCode operator*() { return E; }

  vara::feature::FTErrorCode extractError() { return E; }

  operator bool() const { return false; }

  friend llvm::raw_ostream &
  operator<<(llvm::raw_ostream &OS,
             const Error<vara::feature::FTErrorCode> &Error) {
    switch (Error.E) {
    case vara::feature::ABORTED:
      OS << "Transaction aborted.";
      break;
    case vara::feature::ALREADY_PRESENT:
      OS << "Node already present.";
      break;
    case vara::feature::ERROR:
      OS << "Error.";
      break;
    case vara::feature::INCONSISTENT:
      OS << "Inconsistent feature model.";
      break;
    case vara::feature::MISSING_FEATURE:
      OS << "Missing feature.";
      break;
    case vara::feature::MISSING_PARENT:
      OS << "Missing parent of node.";
      break;
    case vara::feature::MISSING_MODEL:
      OS << "Missing a feature model.";
      break;
    case vara::feature::NON_LEAF_NODE:
      OS << "Not a leaf node.";
      break;
    case vara::feature::CONSTRAINT_MALFORMED:
      OS << "Constraint is malformed.";
    }
    return OS;
  }

private:
  vara::feature::FTErrorCode E;
};

} // namespace vara

#endif // VARA_FEATURE_ERROR_H

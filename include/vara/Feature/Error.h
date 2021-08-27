#ifndef VARA_FEATURE_ERROR_H
#define VARA_FEATURE_ERROR_H

#include "vara/Feature/Result.h"
#include "vara/Utils/VariantUtil.h"

#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <iostream>
#include <type_traits>
#include <variant>

namespace vara::feature {

enum FTErrorCode {
  ERROR,
  ABORTED,
  MISSING_FEATURE,
  MISSING_PARENT,
  ALREADY_PRESENT,
  MISSING_MODEL,
  INCONSISTENT
};

namespace result {

template <>
class Error<FTErrorCode> {
public:
  Error(FTErrorCode E) : E(E) {}

  FTErrorCode operator*() { return E; }

  operator bool() const { return false; }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       const Error<FTErrorCode> &Error) {
    switch (Error.E) {
      // TODO(s9latimm): meaningful error msg
    case vara::feature::ABORTED:
      OS << "ABORTED";
      break;
    case vara::feature::ERROR:
      OS << "ERROR";
      break;
    case vara::feature::MISSING_FEATURE:
      OS << "MISSING_FEATURE";
      break;
    case vara::feature::MISSING_PARENT:
      OS << "MISSING_PARENT";
      break;
    case vara::feature::ALREADY_PRESENT:
      OS << "ALREADY_PRESENT";
      break;
    case vara::feature::MISSING_MODEL:
      OS << "MISSING_MODEL";
      break;
    case vara::feature::INCONSISTENT:
      OS << "INCONSISTENT";
      break;
    }
    return OS;
  }

private:
  FTErrorCode E;
};

} // namespace result

} // namespace vara::feature

#endif // VARA_FEATURE_ERROR_H

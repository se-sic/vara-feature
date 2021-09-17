#ifndef VARA_FEATURE_ERROR_H
#define VARA_FEATURE_ERROR_H

#include "vara/Utils/Result.h"
#include "vara/Utils/VariantUtil.h"

#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <iostream>
#include <type_traits>
#include <variant>

namespace vara::feature {

enum FTErrorCode {
  ABORTED,
  ALREADY_PRESENT,
  ERROR,
  INCONSISTENT,
  MISSING_FEATURE,
  MISSING_MODEL,
  MISSING_PARENT
};

} // namespace vara::feature

template <>
class Error<vara::feature::FTErrorCode> {
public:
  Error(vara::feature::FTErrorCode E) : E(E) {}

  vara::feature::FTErrorCode operator*() { return E; }

  operator bool() const { return false; }

  friend llvm::raw_ostream &
  operator<<(llvm::raw_ostream &OS,
             const Error<vara::feature::FTErrorCode> &Error) {
    switch (Error.E) {
      // TODO(s9latimm): meaningful error msg
    case vara::feature::ABORTED:
      OS << "ABORTED";
      break;
    case vara::feature::ALREADY_PRESENT:
      OS << "ALREADY_PRESENT";
      break;
    case vara::feature::ERROR:
      OS << "ERROR";
      break;
    case vara::feature::INCONSISTENT:
      OS << "INCONSISTENT";
      break;
    case vara::feature::MISSING_FEATURE:
      OS << "MISSING_FEATURE";
      break;
    case vara::feature::MISSING_PARENT:
      OS << "MISSING_PARENT";
      break;
    case vara::feature::MISSING_MODEL:
      OS << "MISSING_MODEL";
      break;
    }
    return OS;
  }

private:
  vara::feature::FTErrorCode E;
};

#endif // VARA_FEATURE_ERROR_H

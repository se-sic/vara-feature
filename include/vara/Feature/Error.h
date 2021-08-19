#ifndef VARA_FEATURE_ERROR_H
#define VARA_FEATURE_ERROR_H

#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <iostream>
#include <type_traits>
#include <variant>

namespace vara::feature {

enum ErrorCode {
  ERROR,
  ABORTED,
  MISSING_FEATURE,
  MISSING_PARENT,
  ALREADY_PRESENT,
  MISSING_MODEL,
  INCONSISTENT
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                     const ErrorCode &EC) {
  switch (EC) {
  // TODO(s9latimm): meaningful error msg
  case ABORTED:
    OS << "ABORTED";
    break;
  case ERROR:
    OS << "ERROR";
    break;
  case MISSING_FEATURE:
    OS << "MISSING_FEATURE";
    break;
  case MISSING_PARENT:
    OS << "MISSING_PARENT";
    break;
  case ALREADY_PRESENT:
    OS << "ALREADY_PRESENT";
    break;
  case MISSING_MODEL:
    OS << "MISSING_MODEL";
    break;
  case INCONSISTENT:
    OS << "INCONSISTENT";
    break;
  }
  return OS;
}

template <typename ValueTy, typename ErrorTy = ErrorCode,
          std::enable_if_t<!std::is_same_v<ValueTy, ErrorTy>, bool> = true>
class ErrorOr {
public:
  ErrorOr(ValueTy Value) : Value(std::move(Value)) {}

  ErrorOr(ErrorTy Error) : Value(std::move(Error)) {}

  /// Return false if there is an error.
  operator bool() const { return !std::holds_alternative<ErrorTy>(Value); }

  [[nodiscard]] ValueTy operator*() {
    assert(std::holds_alternative<ValueTy>(Value));
    return std::move(std::get<ValueTy>(Value));
  }

  [[nodiscard]] ErrorTy getError() {
    assert(std::holds_alternative<ErrorTy>(Value));
    return std::move(std::get<ErrorTy>(Value));
  }

private:
  std::variant<ValueTy, ErrorTy> Value;
};

class Result : public ErrorOr<std::monostate, ErrorCode> {
public:
  Result() : ErrorOr<std::monostate, ErrorCode>(std::monostate{}) {}
  Result(ErrorCode Error) : ErrorOr<std::monostate, ErrorCode>(Error) {}
};

} // namespace vara::feature

#endif // VARA_FEATURE_ERROR_H

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

protected:
  std::variant<ValueTy, ErrorTy> Value;
};

class Result : public ErrorOr<std::monostate, ErrorCode> {
public:
  Result() : ErrorOr<std::monostate, ErrorCode>(std::monostate{}) {}
  Result(ErrorCode Error) : ErrorOr<std::monostate, ErrorCode>(Error) {}

  std::string toString() {
    if (this->operator bool()) {
      return "SUCCESS";
    }
    switch (getError()) {
      // TODO(s9latimm): meaningful error msg
    case vara::feature::ABORTED:
      return "ABORTED";
    case vara::feature::ERROR:
      return "ERROR";
    case vara::feature::MISSING_FEATURE:
      return "MISSING_FEATURE";
    case vara::feature::MISSING_PARENT:
      return "MISSING_PARENT";
    case vara::feature::ALREADY_PRESENT:
      return "ALREADY_PRESENT";
    case vara::feature::MISSING_MODEL:
      return "MISSING_MODEL";
    case vara::feature::INCONSISTENT:
      return "INCONSISTENT";
    }
  }
};

} // namespace vara::feature

#endif // VARA_FEATURE_ERROR_H

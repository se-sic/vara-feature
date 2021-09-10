#ifndef VARA_UTILS_RESULT_H
#define VARA_UTILS_RESULT_H

#include "vara/Utils/VariantUtil.h"

#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <iostream>
#include <type_traits>
#include <variant>

namespace result {

template <typename ValueTy>
class Ok {
public:
  Ok(ValueTy V) : V(std::move(V)) {}

  ValueTy operator*() && { return std::move(V); }

  template <
      typename LValueTy = ValueTy,
      std::enable_if_t<std::is_copy_constructible_v<LValueTy>, bool> = true>
  LValueTy operator*() & {
    return V;
  }

  template <
      typename LValueTy = ValueTy,
      std::enable_if_t<!std::is_copy_constructible_v<LValueTy>, bool> = true>
  LValueTy operator*() & {
    return std::move(V);
  }

  operator bool() const { return true; }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       const Ok<ValueTy> &Ok) {
    OS << Ok.V;
    return OS;
  }

private:
  ValueTy V;
};

template <>
class Ok<void> {
public:
  Ok() = default;

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       const Ok<void> &Ok) {
    OS << "OK";
    return OS;
  }
};

template <typename ErrorTy>
class Error {
public:
  Error(ErrorTy V) : E(std::move(V)) {}

  ErrorTy operator*() && { return std::move(E); }

  template <
      typename LValueTy = ErrorTy,
      std::enable_if_t<std::is_copy_constructible_v<LValueTy>, bool> = true>
  LValueTy operator*() & {
    return E;
  }

  template <
      typename LValueTy = ErrorTy,
      std::enable_if_t<!std::is_copy_constructible_v<LValueTy>, bool> = true>
  LValueTy operator*() & {
    return std::move(E);
  }

  operator bool() const { return false; }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       const Error<ErrorTy> &Error) {
    OS << Error.E;
    return OS;
  }

private:
  ErrorTy E;
};

} // namespace result

template <typename ErrorTy, typename ValueTy = void>
class Result {
public:
  Result(result::Ok<ValueTy> V)
      : Variant(std::forward<result::Ok<ValueTy>>(V)) {}

  Result(result::Error<ErrorTy> E)
      : Variant(std::forward<result::Error<ErrorTy>>(E)) {}

  /// Return false if there is an error.
  operator bool() const {
    return !std::holds_alternative<result::Error<ErrorTy>>(Variant);
  }

  [[nodiscard]] ValueTy operator*() {
    assert(std::holds_alternative<result::Ok<ValueTy>>(Variant));
    return *std::get<result::Ok<ValueTy>>(Variant);
  }

  [[nodiscard]] ErrorTy getError() {
    assert(std::holds_alternative<result::Error<ErrorTy>>(Variant));
    return *std::get<result::Error<ErrorTy>>(Variant);
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       const Result<ErrorTy, ValueTy> &R) {
    std::visit([&OS](auto V) { OS << V; }, R.Variant);
    return OS;
  }

protected:
  std::variant<result::Ok<ValueTy>, result::Error<ErrorTy>> Variant;
};

template <typename ValueTy>
result::Ok<ValueTy> Ok(ValueTy V) {
  return result::Ok<ValueTy>(std::forward<ValueTy>(V));
}

inline result::Ok<void> Ok() { return result::Ok<void>(); }

template <typename ErrorTy>
result::Error<ErrorTy> Error(ErrorTy E) {
  return result::Error<ErrorTy>(std::forward<ErrorTy>(E));
}

#endif // VARA_UTILS_RESULT_H

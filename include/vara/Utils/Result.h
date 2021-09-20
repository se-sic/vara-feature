#ifndef VARA_UTILS_RESULT_H
#define VARA_UTILS_RESULT_H

#include "vara/Utils/VariantUtil.h"

#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <iostream>
#include <type_traits>
#include <variant>

//===----------------------------------------------------------------------===//
//                                      Ok
//===----------------------------------------------------------------------===//

template <typename ValueTy = void>
class Ok {
public:
  Ok(ValueTy V) : V(std::move(V)) {}

  template <typename... ArgsTy>
  Ok(ArgsTy &&...Args) : V(std::forward<ArgsTy>(Args)...) {}

  ValueTy operator*() & { return V; }

  ValueTy operator*() && { return extract_value(); }

  ValueTy extract_value() { return std::move(V); }

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

//===----------------------------------------------------------------------===//
//                                    Error
//===----------------------------------------------------------------------===//

template <typename ErrorTy>
class Error {
public:
  Error(ErrorTy E) : E(std::move(E)) {}

  template <typename... ArgsTy>
  Error(ArgsTy &&...Args) : E(std::forward<ArgsTy>(Args)...) {}

  ErrorTy operator*() & { return E; }

  ErrorTy operator*() && { return extract_error(); }

  ErrorTy extract_error() { return std::move(E); }

  operator bool() const { return false; }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       const Error<ErrorTy> &Error) {
    OS << Error.E;
    return OS;
  }

private:
  ErrorTy E;
};

//===----------------------------------------------------------------------===//
//                                      Result
//===----------------------------------------------------------------------===//

template <typename ErrorTy, typename ValueTy = void>
class Result {
public:
  Result(Ok<ValueTy> V) : Variant(std::move(V)) {}

  Result(Error<ErrorTy> E) : Variant(std::move(E)) {}

  template <typename OtherTy,
            std::enable_if_t<!std::is_same_v<ValueTy, OtherTy>, bool> = true>
  Result(Result<ErrorTy, OtherTy> Other) : Variant(Other.extract_error()) {}

  template <typename... ArgsTy,
            std::enable_if_t<std::is_constructible_v<ValueTy, ArgsTy...>,
                             bool> = true>
  Result(ArgsTy &&...Args)
      : Variant(std::in_place_type<Ok<ValueTy>>,
                std::forward<ArgsTy>(Args)...) {}

  template <typename... ArgsTy,
            std::enable_if_t<std::is_constructible_v<ErrorTy, ArgsTy...>,
                             bool> = true>
  Result(ArgsTy &&...Args)
      : Variant(std::in_place_type<Error<ErrorTy>>,
                std::forward<ArgsTy>(Args)...) {}

  /// Return false if there is an error.
  operator bool() const {
    return !std::holds_alternative<Error<ErrorTy>>(Variant);
  }

  [[nodiscard]] ValueTy operator*() & {
    assert(std::holds_alternative<Ok<ValueTy>>(Variant));
    return *std::get<Ok<ValueTy>>(Variant);
  }

  ValueTy operator*() && { return extract_value(); }

  ValueTy extract_value() {
    assert(std::holds_alternative<Ok<ValueTy>>(Variant));
    return std::get<Ok<ValueTy>>(Variant).extract_value();
  }

  [[nodiscard]] ErrorTy getError() & {
    assert(std::holds_alternative<Error<ErrorTy>>(Variant));
    return *std::get<Error<ErrorTy>>(Variant);
  }

  ErrorTy getError() && { return extract_error(); }

  ErrorTy extract_error() {
    assert(std::holds_alternative<Error<ErrorTy>>(Variant));
    return std::get<Error<ErrorTy>>(Variant).extract_error();
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS,
                                       const Result<ErrorTy, ValueTy> &R) {
    std::visit([&OS](auto V) { OS << V; }, R.Variant);
    return OS;
  }

protected:
  std::variant<Ok<ValueTy>, Error<ErrorTy>> Variant;
};

#endif // VARA_UTILS_RESULT_H

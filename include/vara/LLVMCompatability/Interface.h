#ifndef VARA_LLVMCOMPATABILITY_INTERFACE_H
#define VARA_LLVMCOMPATABILITY_INTERFACE_H

// #define WUFF

#ifdef WUFF

#include "llvm/ADT/STLFunctionalExtras.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm_adapter {

#define IN_LLVM_TREE
using ostream = llvm::raw_ostream;
using StringRef = llvm::StringRef;

inline auto takeWhile(llvm::StringRef str,
                      llvm::function_ref<bool(char)> functor) {
  return str.take_while(functor);
}
inline std::string convertToString(llvm::StringRef str) { return str.str(); }
inline std::string toLower(llvm::StringRef str) { return str.lower(); }

#else

#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <string_view>

namespace llvm_adapter {

using ostream = std::ostream;
using StringRef = std::string_view;

inline auto takeWhile(std::string_view str, std::function<bool(char)> functor) {
  auto endPos = std::distance(
      std::begin(str), std::find_if_not(str.begin(), str.end(), functor));
  return str.substr(0, endPos);
}
inline std::string convertToString(std::string_view str) {
  return std::string{str};
}
inline std::string toLower(std::string_view str) {
  std::string tmp{str};
  std::transform(tmp.begin(), tmp.end(), tmp.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return tmp;
}

#endif

} // namespace llvm_adapter

#endif // VARA_LLVMCOMPATABILITY_INTERFACE_H

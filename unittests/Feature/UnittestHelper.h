#ifndef UNITTEST_HELPER_H
#define UNITTEST_HELPER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"

inline std::string getTestResource(llvm::StringRef ResourcePath = "") {
  constexpr const char *BasePath = "../resources/";
  return (llvm::Twine(BasePath) + ResourcePath).str();
}

#endif // UNITTEST_HELPER_H

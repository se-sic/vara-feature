#ifndef UNITTEST_HELPER_H
#define UNITTEST_HELPER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"

inline std::string getTestResource(llvm::StringRef ResourcePath = "") {
  constexpr const char *BasePath = "resources/";
  return (llvm::Twine(BasePath) + ResourcePath).str();
}

namespace vara::feature {

template <class ConstraintTy>
std::unique_ptr<ConstraintTy> createBinaryConstraint(const std::string &A,
                                                     const std::string &B) {
  return std::make_unique<ConstraintTy>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>(A)),
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>(B)));
}

} // namespace vara::feature

#endif // UNITTEST_HELPER_H

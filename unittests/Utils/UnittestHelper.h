#ifndef UNITTEST_HELPER_H
#define UNITTEST_HELPER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"

inline std::string getTestResource(llvm::StringRef ResourcePath = "") {
  constexpr const char *BasePath = "resources/";
  return (llvm::Twine(BasePath) + ResourcePath).str();
}

namespace vara::feature {

template <class OperatorTy>
std::unique_ptr<FeatureModel::BooleanConstraint>
createBinaryConstraint(const std::string &A, const std::string &B) {
  return std::make_unique<FeatureModel::BooleanConstraint>(
      std::make_unique<OperatorTy>(std::make_unique<PrimaryFeatureConstraint>(
                                       std::make_unique<BinaryFeature>(A)),
                                   std::make_unique<PrimaryFeatureConstraint>(
                                       std::make_unique<BinaryFeature>(B))));
}

} // namespace vara::feature

#endif // UNITTEST_HELPER_H

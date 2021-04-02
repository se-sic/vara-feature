#include "vara/Feature/FeatureModelBuilder.h"

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                        FeatureModelBuilder
//===----------------------------------------------------------------------===//

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildFeatureModel() {
  assert(FM->getRoot() && "FeatureModel has no root.");

  if (!FeatureBuilder.commit()) {
    llvm::errs() << "Building features failed.";
    return nullptr;
  }
  if (!ModelBuilder.commit()) {
    llvm::errs() << "Building feature tree failed.";
    return nullptr;
  }
  if (!RelationBuilder.commit()) {
    llvm::errs() << "Building feature relations failed.";
    return nullptr;
  }
  return std::move(FM);
}

} // namespace vara::feature

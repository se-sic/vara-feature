#include "vara/Feature/FeatureModelBuilder.h"

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                        FeatureModelBuilder
//===----------------------------------------------------------------------===//

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildFeatureModel() {
  assert(FM->getRoot() && "FeatureModel has no root.");

  if (Result E = FeatureBuilder.commit(); !E) {
    llvm::errs() << "Building features failed with error " << E;
    return nullptr;
  }
  if (Result E = ModelBuilder.commit(); !E) {
    llvm::errs() << "Building feature tree failed with error " << E;
    return nullptr;
  }
  if (Result E = RelationBuilder.commit(); !E) {
    llvm::errs() << "Building feature relations failed with error " << E;
    return nullptr;
  }
  return std::move(FM);
}

} // namespace vara::feature

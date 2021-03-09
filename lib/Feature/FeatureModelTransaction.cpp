#include "vara/Feature/FeatureModelTransaction.h"

#include <iostream>

namespace vara::feature {

bool mergeSubtree(FeatureModelCopyTransaction &Trans, FeatureModel &FM,
                  Feature &F);
std::unique_ptr<Feature> FeatureSoftCopy(Feature *F);
bool FeatureSoftCompare(const Feature &F1, const Feature &F2);

void addFeature(FeatureModel &FM, std::unique_ptr<Feature> NewFeature,
                Feature *Parent) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  Trans.addFeature(std::move(NewFeature), Parent);
  Trans.commit();
}

[[nodiscard]] std::unique_ptr<FeatureModel>
mergeFeatureModels(FeatureModel &FM1, FeatureModel &FM2) {
  auto Trans = FeatureModelCopyTransaction::openTransaction(FM1);
  if (!mergeSubtree(Trans, FM1, *FM2.getRoot())) {
    return nullptr;
  } else {
    return Trans.commit();
  }
}

[[nodiscard]] bool mergeSubtree(FeatureModelCopyTransaction &Trans,
                                FeatureModel &FM, Feature &F) {
  // Is there a similar Feature in the other FM
  if (Feature *CMP = FM.getFeature(F.getName())) {
    if (FeatureSoftCompare(*CMP, F)) {
      // similar feature, maybe merge locations
      for (FeatureSourceRange &FSR : F.getLocations()) {
        if (std::find(CMP->getLocationsBegin(), CMP->getLocationsEnd(), FSR) ==
            CMP->getLocationsEnd()) {
          CMP->addLocation(FSR);
        }
      }
    } else {
      Trans.abort();
      return false;
    }
  } else {
    std::unique_ptr<Feature> SoftCopy = FeatureSoftCopy(&F);
    if (!SoftCopy) {
      Trans.abort();
      return false;
    }
    Trans.addFeature(std::move(SoftCopy), F.getParentFeature());
  }

  // copy children  if missing
  for (Feature *Child : F.getChildren<Feature>()) {
    if (!mergeSubtree(Trans, FM, *Child)) {
      // aborted in callee
      return false;
    }
  }
  return true;
}

[[nodiscard]] std::unique_ptr<Feature> FeatureSoftCopy(Feature *F) {
  std::unique_ptr<Feature> FeatureSoftCopy;
  NumericFeature::ValuesVariantType Values;
  switch (F->getKind()) {
  case Feature::FeatureKind::FK_BINARY:
    return std::make_unique<BinaryFeature>(
        F->getName(), F->isOptional(),
        std::vector<FeatureSourceRange>(F->getLocationsBegin(),
                                        F->getLocationsEnd()));
  case Feature::FeatureKind::FK_NUMERIC:
    Values = dynamic_cast<NumericFeature *>(F)->getValues();
    return std::make_unique<NumericFeature>(
        F->getName(), Values, F->isOptional(),
        std::vector<FeatureSourceRange>(F->getLocationsBegin(),
                                        F->getLocationsEnd()));
  case Feature::FeatureKind::FK_ROOT:
    return std::make_unique<RootFeature>();
  case Feature::FeatureKind::FK_UNKNOWN:
    return nullptr;
  }
}

[[nodiscard]] bool FeatureSoftCompare(const Feature &F1, const Feature &F2) {
  assert(F1.getName() == F2.getName());
  if (F1.getKind() != F2.getKind()) {
    return false;
  }
  if (F1.isOptional() != F2.isOptional()) {
    return false;
  }
  if (F1.getKind() == Feature::FeatureKind::FK_ROOT) {
    return true;
  }
  if (*(F1.getParentFeature()) != *(F2.getParentFeature())) {
    return false;
  }
  if (F1.getParent()->getKind() != F2.getParent()->getKind()) {
    return false;
  }
  if (F1.getKind() == Feature::FeatureKind::FK_BINARY) {
    return true;
  }
  if (const auto *NF1 = llvm::dyn_cast<NumericFeature>(&F1)) {
    if (const auto *NF2 = llvm::dyn_cast<NumericFeature>(&F2)) {
      return NF1->getValues() == NF2->getValues();
    }
  }
  return false;
}

} // namespace vara::feature

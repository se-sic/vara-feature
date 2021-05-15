#include "vara/Feature/FeatureModelTransaction.h"

#include <iostream>

namespace vara::feature {

bool mergeSubtree(FeatureModelCopyTransaction &Trans, FeatureModel const &FM,
                  Feature &F);
std::unique_ptr<Feature> FeatureCopy(Feature &F);
bool CompareProperties(const Feature &F1, const Feature &F2);

void addFeature(FeatureModel &FM, std::unique_ptr<Feature> NewFeature,
                Feature *Parent) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  Trans.addFeature(std::move(NewFeature), Parent);
  Trans.commit();
}

void addFeatures(
    FeatureModel &FM,
    std::vector<std::pair<std::unique_ptr<Feature>, Feature *>> NewFeatures) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  for (auto &NewFeature : NewFeatures) {
    Trans.addFeature(std::move(NewFeature.first), NewFeature.second);
  }
  Trans.commit();
}

void removeFeature(FeatureModel &FM,
                   detail::FeatureVariantTy FeatureToBeDeleted,
                   bool Recursive) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  Trans.removeFeature(FeatureToBeDeleted, Recursive);
  Trans.commit();
}

void removeFeatures(FeatureModel &FM,
                    std::vector<detail::FeatureVariantTy> FeaturesToBeDeleted,
                    bool Recursive) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);

  // create mapping of <Parent, Child> to decide later if a Feature has a child
  // assumption: every parent can only have one child --> Is this correct?
  std::map<Feature *, Feature *> ParentChildMapping;
  std::map<Feature *, Feature *> ChildParentMapping;
  for (auto FeatureVariant : FeaturesToBeDeleted) {
    Feature *ActualFeature = get_if<Feature *>(&FeatureVariant)
                                 ? get<Feature *>(FeatureVariant)
                                 : FM.getFeature(get<string>(FeatureVariant));
    if (ActualFeature->getParentFeature()) {
      ParentChildMapping.insert(
          std::make_pair(ActualFeature->getParentFeature(), ActualFeature));
      ChildParentMapping.insert(
          std::make_pair(ActualFeature, ActualFeature->getParentFeature()));
    }
  }

  std::vector<detail::FeatureVariantTy> CopiedFeatures;
  std::copy(FeaturesToBeDeleted.begin(), FeaturesToBeDeleted.end(),
            CopiedFeatures);

  while (!CopiedFeatures.empty()) {
    std::vector<detail::FeatureVariantTy> RemainingFeatures;
    std::copy(CopiedFeatures.begin(), CopiedFeatures.end(), RemainingFeatures);
    // loop over all Leaves/Feature until they are all gone
    for (auto FeatureIterator = CopiedFeatures.begin();
         FeatureIterator != CopiedFeatures.end(); ++FeatureIterator) {
      // if we truly have a Leave/Feature --> current Feature is not a parent
      Feature *ActualFeature =
          get_if<Feature *>(&(*FeatureIterator))
              ? get<Feature *>((*FeatureIterator))
              : FM.getFeature(get<string>((*FeatureIterator)));
      // if Feature is not a parent
      if (ParentChildMapping.find(ActualFeature) == ParentChildMapping.end()) {
        Trans.removeFeature(*FeatureIterator, Recursive);
        RemainingFeatures.erase(FeatureIterator);
        ParentChildMapping.erase(ChildParentMapping[ActualFeature]);
      }
    }
    std::copy(RemainingFeatures.begin(), RemainingFeatures.end(),
              CopiedFeatures);
  }
  Trans.commit();
}

void setCommit(FeatureModel &FM, std::string NewCommit) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  Trans.setCommit(std::move(NewCommit));
  Trans.commit();
}

[[nodiscard]] std::unique_ptr<FeatureModel>
mergeFeatureModels(FeatureModel &FM1, FeatureModel &FM2) {
  auto Trans = FeatureModelCopyTransaction::openTransaction(FM1);
  if (!mergeSubtree(Trans, FM1, *FM2.getRoot())) {
    Trans.abort();
    return nullptr;
  }
  return Trans.commit();
}

[[nodiscard]] bool mergeSubtree(FeatureModelCopyTransaction &Trans,
                                FeatureModel const &FM, Feature &F) {
  // Is there a similar Feature in the original FM
  if (Feature *CMP = FM.getFeature(F.getName())) {
    if (CompareProperties(*CMP, F)) {
      // similar feature, maybe merge locations
      for (FeatureSourceRange const &FSR : F.getLocations()) {
        if (std::find(CMP->getLocationsBegin(), CMP->getLocationsEnd(), FSR) ==
            CMP->getLocationsEnd()) {
          Trans.addLocation(CMP, FSR);
        }
      }
    } else {
      return false;
    }
  } else {
    std::unique_ptr<Feature> Copy = FeatureCopy(F);
    if (!Copy) {
      return false;
    }
    Trans.addFeature(std::move(Copy), F.getParentFeature());
  }

  // copy children if missing
  for (Feature *Child : F.getChildren<Feature>()) {
    if (!mergeSubtree(Trans, FM, *Child)) {
      return false;
    }
  }
  return true;
}

[[nodiscard]] std::unique_ptr<Feature> FeatureCopy(Feature &F) {
  switch (F.getKind()) {
  case Feature::FeatureKind::FK_BINARY:
    return std::make_unique<BinaryFeature>(
        F.getName().str(), F.isOptional(),
        std::vector<FeatureSourceRange>(F.getLocationsBegin(),
                                        F.getLocationsEnd()));
  case Feature::FeatureKind::FK_NUMERIC:
    if (auto *NF = llvm::dyn_cast<NumericFeature>(&F)) {
      return std::make_unique<NumericFeature>(
          F.getName().str(), NF->getValues(), F.isOptional(),
          std::vector<FeatureSourceRange>(F.getLocationsBegin(),
                                          F.getLocationsEnd()));
    }
  case Feature::FeatureKind::FK_ROOT:
    return std::make_unique<RootFeature>(F.getName().str());
  default:
    break;
  }
  return nullptr;
}

[[nodiscard]] bool CompareProperties(const Feature &F1, const Feature &F2) {
  if (F1.getName() != F2.getName()) {
    return false;
  }
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

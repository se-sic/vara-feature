#include "vara/Feature/FeatureModelTransaction.h"

#include <iostream>

namespace vara::feature {

bool mergeSubtree(FeatureModelCopyTransaction &Trans, FeatureModel const &FM,
                  Feature &F, bool Strict);

bool mergeSubtree(FeatureModelCopyTransaction &Trans, FeatureModel const &FM,
                  Relationship &R, bool Strict);

std::unique_ptr<Feature> FeatureCopy(Feature &F);

bool CompareProperties(const Feature &F1, const Feature &F2, bool Strict);

std::optional<Relationship *> getFeatureRelationship(Feature &F);

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

void addRelationship(FeatureModel &FM,
                     const detail::FeatureVariantTy &GroupRoot,
                     Relationship::RelationshipKind Kind) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  Trans.addRelationship(Kind, GroupRoot);
  Trans.commit();
}

void removeRelationship(FeatureModel &FM,
                        const detail::FeatureVariantTy &GroupRoot) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  Trans.removeRelationship(GroupRoot);
  Trans.commit();
}

void setCommit(FeatureModel &FM, std::string NewCommit) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  Trans.setCommit(std::move(NewCommit));
  Trans.commit();
}

[[nodiscard]] std::unique_ptr<FeatureModel>
mergeFeatureModels(FeatureModel &FM1, FeatureModel &FM2, bool Strict) {
  auto Trans = FeatureModelCopyTransaction::openTransaction(FM1);
  if (!mergeSubtree(Trans, FM1, *FM2.getRoot(), Strict)) {
    Trans.abort();
    return nullptr;
  }
  return Trans.commit();
}

[[nodiscard]] bool mergeSubtree(FeatureModelCopyTransaction &Trans,
                                FeatureModel const &FM, Feature &F,
                                bool Strict) {
  // Is there a similar Feature in the original FM
  if (Feature *CMP = FM.getFeature(F.getName())) {
    if (CompareProperties(*CMP, F, Strict)) {
      // similar feature, maybe add relationship
      auto FRelationship = getFeatureRelationship(F);
      auto CMPRelationship = getFeatureRelationship(*CMP);
      if (FRelationship && CMPRelationship) {
        // Relationships must match
        if (FRelationship.value()->getKind() !=
            CMPRelationship.value()->getKind()) {
          return false;
        }
      } else if (FRelationship && !CMPRelationship) {
        if (CMP->getChildren<Feature>().size() < 2) {
          Trans.addRelationship(FRelationship.value()->getKind(),
                                F.getName().str());
        } else {
          return false;
        }
      }
      // CMPRelationship is already in the new model

      // merge locations
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
    // add relationship to new feature
    auto Relationship = getFeatureRelationship(F);
    if (Relationship.has_value()) {
      Trans.addRelationship(Relationship.value()->getKind(), F.getName());
    }
  }

  for (Feature *Child : F.getChildren<Feature>()) {
    if (!mergeSubtree(Trans, FM, *Child, Strict)) {
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

[[nodiscard]] bool CompareProperties(const Feature &F1, const Feature &F2,
                                     bool Strict) {
  // name equality
  if (F1.getName() != F2.getName()) {
    return false;
  }
  // parent equality, properties checked before so name equality is sufficient
  if (F1.getKind() != Feature::FeatureKind::FK_ROOT &&
      F2.getKind() != Feature::FeatureKind::FK_ROOT) {
    if (*(F1.getParentFeature()) != *(F2.getParentFeature())) {
      return false;
    }
  }
  if (!Strict) {
    // strict merging still requires equal parent names, otherwise we might
    // introduce implicit constraint due to model structure, e.g., merging group
    return true;
  }
  if (F1.isOptional() != F2.isOptional()) {
    return false;
  }
  if (F1.getKind() != F2.getKind()) {
    return false;
  }
  if (F1.getKind() == Feature::FeatureKind::FK_ROOT) {
    return true;
  }
  //  if (F1.getParent()->getKind() != F2.getParent()->getKind()) {
  //    return false;
  //  }
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

std::optional<Relationship *> getFeatureRelationship(Feature &F) {
  auto Relationships = F.getChildren<Relationship>();
  if (Relationships.empty()) {
    return std::nullopt;
  }
  return *(Relationships.begin());
}

} // namespace vara::feature

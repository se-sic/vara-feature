#include "vara/Feature/FeatureModelTransaction.h"
#include "vara/Utils/VariantUtil.h"

#include <algorithm>
#include <iostream>
#include <vector>

namespace vara::feature {

bool mergeSubtree(FeatureModelCopyTransaction &Trans, FeatureModel const &FM,
                  Feature &F, bool Strict);

std::unique_ptr<Feature> FeatureCopy(Feature &F);

bool CompareProperties(const Feature &F1, const Feature &F2, bool Strict);

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

Feature *getActualFeature(FeatureModel &FM, detail::FeatureVariantTy &FV) {
  Feature *ActualFeature = nullptr;
  std::visit(
      Overloaded{[&ActualFeature, &FM](Feature *F) { ActualFeature = F; },
                 [&ActualFeature, &FM](string &FName) {
                   ActualFeature = FM.getFeature(FName);
                 }},
      FV);
  return ActualFeature;
}

std::optional<bool> fvIsLeave(FeatureModel &FM, detail::FeatureVariantTy &FV) {
  Feature *ActualFeature = getActualFeature(FM, FV);
  // if Feature does not exist in FM
  if (ActualFeature == nullptr) {
    return std::nullopt;
  }
  return std::optional<bool>{ActualFeature->isLeave()};
}

/// Checks if in recursive mode the provided Feature can be deleted or not.
///
/// \param FM
/// \param FV
/// \param OtherFeatures
/// \return optional of true, if all transitive children of FV are not part of
/// OtherFeatures; optional of false, if at least one transitive child of FV is
/// part of OtherFeatures; nullopt, if FV is not part of FM
std::optional<bool>
canBeDeletedRecursively(FeatureModel &FM, detail::FeatureVariantTy &FV,
                        const std::set<FeatureTreeNode *> &OtherFeatures) {
  Feature *ActualFeature = getActualFeature(FM, FV);
  if (ActualFeature == nullptr) { // Feature does not exist in FM
    return std::nullopt;
  }

  std::set<FeatureTreeNode *> Intersection;
  auto AllChildrenInSubtree = ActualFeature->getChildren();
  std::set_intersection(
      AllChildrenInSubtree.begin(), AllChildrenInSubtree.end(),
      OtherFeatures.begin(), OtherFeatures.end(),
      std::inserter(Intersection, std::next(Intersection.begin())));
  // if the children are not matching the other features, the intersection is
  // empty and we are good to go for deletion
  return std::optional<bool>{Intersection.begin() == Intersection.end()};
}

std::vector<detail::FeatureVariantTy> removeFeatures(
    FeatureModel &FM, std::vector<detail::FeatureVariantTy>::iterator Begin,
    std::vector<detail::FeatureVariantTy>::iterator End, bool Recursive) {
  // if everything was deleted
  std::vector<detail::FeatureVariantTy> NotDeletedFeatures;
  if (Begin == End) {
    return NotDeletedFeatures;
  }

  // 3 use-cases:
  // If Feature is Leave --> can be deleted
  // If Feature is not a Leave and !Recursive --> cannot be deleted
  // If Recursive --> partition by "youngest" feature in sub-tree --> delete
  // those in recursive mode
  auto DeleteIt = std::partition(
      Begin, End, [&FM, Recursive, &Begin, &End](detail::FeatureVariantTy &FV) {
        if (fvIsLeave(FM, FV).value_or(false)) {
          return true;
        }
        if (!Recursive) {
          return false;
        }
        std::set<FeatureTreeNode *> OtherFeatures;
        std::transform(
            Begin, End,
            std::inserter(OtherFeatures, std::next(OtherFeatures.begin())),
            [&FM, &OtherFeatures](detail::FeatureVariantTy &FV) {
              return getActualFeature(FM, FV);
            });
        // Remove nullptr, in case, on of the specified Features could 
        // not be found in the FeatureModel 
        OtherFeatures.erase(nullptr);
        return canBeDeletedRecursively(FM, FV, OtherFeatures).value_or(false);
      });

  // check if something can be deleted --> if not and return non-deletable
  // features
  if (DeleteIt == Begin) {
    return std::vector(Begin, End);
  }

  // perform actual removal of deletable features
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  auto it = Begin;
  while (it != DeleteIt) {
    Trans.removeFeature(*it, Recursive);
    it = std::next(it);
  }
  Trans.commit();

  // call remove Features on non-leaves
  return removeFeatures(FM, DeleteIt, End, Recursive);
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
  if (F1.getName() != F2.getName()) {
    return false;
  }
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

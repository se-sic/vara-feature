#include "vara/Feature/FeatureModelTransaction.h"

#include "vara/Utils/VariantUtil.h"

#include <algorithm>
#include <iostream>
#include <vector>

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
  // Iterate over Features and deleted them iff they are leaves (don't have
  // children).
  // Even if we are in the recursive mode, we need to delete the children among
  // FeaturesToBeDeleted first. Otherwise, the commit gets stuck in an endless
  // loop.
  std::vector<detail::FeatureVariantTy> RemainingFeatures;
  std::copy(FeaturesToBeDeleted.begin(), FeaturesToBeDeleted.end(),
            std::back_inserter(RemainingFeatures));
  while (!RemainingFeatures.empty()) {
    auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
    std::vector<detail::FeatureVariantTy> DeleteFeatures;
    for (auto &RemainingFeature : RemainingFeatures) {
      Feature *ActualFeature;
      std::visit(
          Overloaded{[&ActualFeature, &FM](Feature *F) { ActualFeature = F; },
                     [&ActualFeature, &FM](string &F) {
                       ActualFeature = FM.getFeature(F);
                     }},
          RemainingFeature);
      // if Feature is not a parent it can be deleted
      if (ActualFeature->children().empty()) {
        Trans.removeFeature(RemainingFeature, Recursive);
        DeleteFeatures.emplace_back(RemainingFeature);
      }
    }
    // We are not in recursive mode and have only parents left with children
    // that should not be deleted --> abort execution
    if (DeleteFeatures.empty()) {
      Trans.commit();
      if (Recursive) {
        std::sort(RemainingFeatures.begin(), RemainingFeatures.end(),
                  [&FM](detail::FeatureVariantTy &FV1,
                        detail::FeatureVariantTy &FV2) -> bool {
                    Feature *ActualFeature1;
                    std::visit(Overloaded{[&ActualFeature1, &FM](Feature *F) {
                                            ActualFeature1 = F;
                                          },
                                          [&ActualFeature1, &FM](string &F) {
                                            ActualFeature1 = FM.getFeature(F);
                                          }},
                               FV1);
                    Feature *ActualFeature2;
                    std::visit(Overloaded{[&ActualFeature2, &FM](Feature *F) {
                                            ActualFeature2 = F;
                                          },
                                          [&ActualFeature2, &FM](string &F) {
                                            ActualFeature2 = FM.getFeature(F);
                                          }},
                               FV2);

                    return countNumberOfSuccessors(ActualFeature1) <
                           countNumberOfSuccessors(ActualFeature2);
                  });
        removeFeaturesRecursively(FM, RemainingFeatures);
      }
      // TODO: return list of non-deletable Features?
      return;
    }

    std::sort(DeleteFeatures.begin(), DeleteFeatures.end());
    RemainingFeatures.erase(
        std::remove_if(RemainingFeatures.begin(), RemainingFeatures.end(),
                       [&](auto X) {
                         return binary_search(DeleteFeatures.begin(),
                                              DeleteFeatures.end(), X);
                       }),
        RemainingFeatures.end());
    Trans.commit();
  }
}

int countNumberOfSuccessors(FeatureTreeNode *F) {
  if (F->children().empty()) {
    return 0;
  }
  auto Count = std::distance(F->children().begin(), F->children().end());

  // count children of children
  for (auto *Child : F->children()) {
    Count += countNumberOfSuccessors(Child);
  }
  return Count;
}

void removeFeaturesRecursively(
    FeatureModel &FM,
    const std::vector<detail::FeatureVariantTy> &FeaturesToBeDeleted) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  for (detail::FeatureVariantTy FeatureVariant : FeaturesToBeDeleted) {
    Trans.removeFeature(FeatureVariant, true);
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

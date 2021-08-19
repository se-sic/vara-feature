#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelBuilder.h"

#include "llvm/Support/Casting.h"

#include <algorithm>

//===----------------------------------------------------------------------===//
//                          FeatureModel
//===----------------------------------------------------------------------===//

namespace vara::feature {
void FeatureModel::dump() const {
  for (const auto &Feature : Features) {
    llvm::outs() << "{\n";
    Feature.second->dump();
    llvm::outs() << "}\n";
  }
  llvm::outs() << '\n';
}

Feature *FeatureModel::addFeature(std::unique_ptr<Feature> NewFeature) {
  auto PosInsertedFeature = Features.try_emplace(
      std::string(NewFeature->getName()), std::move(NewFeature));
  if (!PosInsertedFeature.second) {
    return nullptr;
  }
  return PosInsertedFeature.first->getValue().get();
}

void FeatureModel::removeFeature(Feature &F) {
  if (&F == Root) {
    Root = nullptr;
  }
  Features.erase(F.getName());
}

RootFeature *FeatureModel::setRoot(RootFeature &NewRoot) {
  return Root = &NewRoot;
}

std::unique_ptr<FeatureModel> FeatureModel::clone() const {
  FeatureModelBuilder FMB;
  FMB.setVmName(this->getName().str());
  FMB.setCommit(this->getCommit().str());
  FMB.setPath(this->getPath().string());

  for (const auto &KV : this->Features) {
    std::vector<FeatureSourceRange> SourceRanges(
        KV.getValue()->getLocations().begin(),
        KV.getValue()->getLocations().end());

    switch (KV.getValue()->getKind()) {
    case Feature::FeatureKind::FK_UNKNOWN:
      FMB.makeFeature<Feature>(KV.getValue()->getName().str());
      break;
    case Feature::FeatureKind::FK_ROOT:
      FMB.makeRoot(KV.getValue()->getName().str());
      break;
    case Feature::FeatureKind::FK_BINARY:
      if (auto *F = llvm::dyn_cast<BinaryFeature>(KV.getValue().get()); F) {
        FMB.makeFeature<BinaryFeature>(F->getName().str(), F->isOptional(),
                                       std::move(SourceRanges));
      }
      break;
    case Feature::FeatureKind::FK_NUMERIC:
      if (auto *F = llvm::dyn_cast<NumericFeature>(KV.getValue().get()); F) {
        FMB.makeFeature<NumericFeature>(F->getName().str(), F->getValues(),
                                        F->isOptional(),
                                        std::move(SourceRanges));
      }
      break;
    }

    if (auto *ParentFeature = KV.getValue()->getParentFeature()) {
      FMB.addEdge(ParentFeature->getName().str(),
                  KV.getValue()->getName().str());
    }
  }

  for (const auto &Rel : this->Relationships) {
    if (auto *P = llvm::dyn_cast_or_null<Feature>(Rel->getParent())) {
      FMB.emplaceRelationship(Rel->getKind(), P->getName().str());
    }
  }

  // We can use recursive cloning on constraints, as we can rely on the
  // builder to update pointers into the feature model correctly if invoked
  // afterwards.
  // TODO(s9latimm): Add unittests for cloned Constraints
  for (const auto &C : this->Constraints) {
    FMB.addConstraint(C->clone());
  }

  auto FM = FMB.buildFeatureModel();
  assert(FM);
  return FM;
}

} // namespace vara::feature

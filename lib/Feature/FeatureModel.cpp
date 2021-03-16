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
  auto *InsertedFeature = PosInsertedFeature.first->getValue().get();
  assert(InsertedFeature);
  OrderedFeatures.insert(InsertedFeature);
  return InsertedFeature;
}

Relationship *
FeatureModel::addRelationship(std::unique_ptr<RelationshipTy> Relationship) {
  Relationships.push_back(std::move(Relationship));
  return Relationships.back().get();
}

void FeatureModel::removeFeature(Feature &F) {
  if (&F == Root) {
    Root = nullptr;
  }
  OrderedFeatures.remove(&F);
  Features.erase(F.getName());
}

RootFeature *FeatureModel::setRoot(RootFeature &NewRoot) {
  return Root = &NewRoot;
}

std::unique_ptr<FeatureModel> FeatureModel::clone() {
  FeatureModelBuilder FMB;
  FMB.setVmName(this->getName().str());
  FMB.setCommit(this->getCommit().str());
  FMB.setPath(this->getPath().string());

  for (const auto &KV : this->Features) {
    // TODO(s9latimm): Add unittests for cloned FeatureSourceRanges
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
    std::vector<std::string> FeatureNames;
    for (auto *Child : Rel->children()) {
      if (auto *ChildFeature = llvm::dyn_cast<Feature>(Child)) {
        FeatureNames.emplace_back(ChildFeature->getName());
      }
    }
    if (auto *ParentFeature = llvm::dyn_cast<Feature>(Rel->getParent())) {
      FMB.emplaceRelationship(Rel->getKind(), FeatureNames,
                              ParentFeature->getName().str());
    }
  }

  // We can use recursive cloning on constraints, as we can rely on the
  // builder to update pointers into the feature model correctly if invoked
  // afterwards.
  // TODO(s9latimm): Add unittests for cloned Constraints
  for (const auto &C : this->Constraints) {
    FMB.addConstraint(C->clone());
  }

  return FMB.buildFeatureModel();
}

} // namespace vara::feature

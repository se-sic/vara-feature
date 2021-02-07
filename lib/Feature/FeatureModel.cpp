#include "vara/Feature/FeatureModel.h"

#include "llvm/Support/Casting.h"

#include <algorithm>

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

std::unique_ptr<FeatureModel> FeatureModel::clone() {
  FeatureModelBuilder FMB;
  FMB.setVmName(this->getName().str());
  FMB.setCommit(this->getCommit().str());
  FMB.setPath(this->getPath().string());
  FMB.setRoot(this->getRoot()->getName().str());

  for (const auto &KV : this->Features) {
    // TODO(s9latimm): Add unittests for cloned FeatureSourceRanges
    std::vector<FeatureSourceRange> SourceRanges(
        KV.getValue()->getLocations().begin(),
        KV.getValue()->getLocations().end());

    switch (KV.getValue()->getKind()) {
    case Feature::FeatureKind::FK_UNKNOWN:
      FMB.makeFeature<Feature>(KV.getValue()->getName().str());
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

/// Decide whether two features are mutual exclusive. Beware that this method
/// only detects very simple trees with binary excludes.
bool isSimpleMutex(const Feature *A, const Feature *B) {
  return std::any_of(
      A->excludes().begin(), A->excludes().end(), [A, B](const auto *E) {
        if (const auto *LHS =
                llvm::dyn_cast<PrimaryFeatureConstraint>(E->getLeftOperand());
            LHS) {
          if (const auto *RHS = llvm::dyn_cast<PrimaryFeatureConstraint>(
                  E->getRightOperand());
              RHS) {
            // A excludes B
            if (LHS->getFeature() &&
                LHS->getFeature()->getName() == A->getName() &&
                RHS->getFeature() &&
                RHS->getFeature()->getName() == B->getName()) {
              return std::any_of(
                  B->excludes().begin(), B->excludes().end(),
                  [A, B](const auto *E) {
                    if (const auto *LHS =
                            llvm::dyn_cast<PrimaryFeatureConstraint>(
                                E->getLeftOperand());
                        LHS) {
                      if (const auto *RHS =
                              llvm::dyn_cast<PrimaryFeatureConstraint>(
                                  E->getRightOperand());
                          RHS) {
                        // B excludes A
                        return LHS->getFeature() &&
                               LHS->getFeature()->getName() == B->getName() &&
                               RHS->getFeature() &&
                               RHS->getFeature()->getName() == A->getName();
                      }
                    }
                    return false;
                  });
            }
          }
        }
        return false;
      });
}

/// Collect mutual exclusive feature constraints for clean up.
llvm::SmallSet<Constraint *, 3>
cleanUpMutualExclusiveConstraints(const Feature *A,
                                  const llvm::SmallSet<Feature *, 3> &Xor) {
  llvm::SmallSet<Constraint *, 3> Remove;
  for (const auto *E : A->excludes()) {
    if (auto *LHS =
            llvm::dyn_cast<PrimaryFeatureConstraint>(E->getLeftOperand());
        LHS) {
      if (auto *RHS =
              llvm::dyn_cast<PrimaryFeatureConstraint>(E->getRightOperand());
          RHS) {
        if (LHS->getFeature() && LHS->getFeature()->getName() == A->getName() &&
            RHS->getFeature() && Xor.count(RHS->getFeature())) {
          Remove.insert(LHS);
        } else if (LHS->getFeature() && Xor.count(LHS->getFeature()) &&
                   RHS->getFeature() &&
                   RHS->getFeature()->getName() == A->getName()) {
          Remove.insert(RHS);
        }
      }
    }
  }
  return Remove;
}

void FeatureModelBuilder::detectXMLAlternatives() {
  for (const auto &FeatureName : Features.keys()) {
    std::vector<std::string> Frontier(Children[FeatureName].begin(),
                                      Children[FeatureName].end());
    while (!Frontier.empty()) {
      const auto &FName = Frontier.back();
      Frontier.pop_back();
      if (auto *F = llvm::dyn_cast<Feature>(Features[FName].get());
          F && !F->isOptional()) {
        llvm::SmallSet<Feature *, 3> Xor;
        Xor.insert(F);
        for (const auto &Name : Frontier) {
          if (auto *E = llvm::dyn_cast<Feature>(Features[Name].get());
              E && !E->isOptional()) {
            if (std::all_of(Xor.begin(), Xor.end(), [E](const auto *F) {
                  return isSimpleMutex(F, E);
                })) {
              Xor.insert(E);
            }
          }
        }
        if (Xor.size() > 1) {
          std::vector<std::string> V;
          for (auto *E : Xor) {
            Frontier.erase(std::remove(Frontier.begin(), Frontier.end(),
                                       E->getName().str()),
                           Frontier.end());
            V.push_back(E->getName().str());
            for (auto *R : cleanUpMutualExclusiveConstraints(E, Xor)) {
              E->removeConstraintNonPreserve(R);
            }
          }
          emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, V,
                              FeatureName.str());
        }
      }
    }
  }
}

bool FeatureModelBuilder::buildConstraints() {
  auto B = BuilderVisitor(this);
  for (const auto &C : Constraints) {
    C->accept(B);
  }
  detectXMLAlternatives();
  return true;
}

bool FeatureModelBuilder::buildTree(const string &FeatureName,
                                    std::set<std::string> &Visited) {
  if (find(Visited.begin(), Visited.end(), FeatureName) != Visited.end()) {
    llvm::errs() << "error: Cycle or duplicate edge in \'" << FeatureName
                 << "\'.\n";
    return false;
  }
  Visited.insert(FeatureName);

  if (find(Features.keys().begin(), Features.keys().end(), FeatureName) ==
      Features.keys().end()) {
    llvm::errs() << "error: Missing feature \'\'" << FeatureName << "\'.\n";
    return false;
  }

  for (const auto &Child : Children[FeatureName]) {
    if (Parents[Child] != FeatureName) {
      llvm::errs() << "error: Parent of \'" << Child << "\' does not match \'"
                   << FeatureName << "\'.\n";
      return false;
    }
    if (!buildTree(Child, Visited)) {
      return false;
    }
  }

  llvm::SmallSet<std::string, 3> Skip;
  if (RelationshipEdges.find(FeatureName) != RelationshipEdges.end()) {
    for (const auto &Pair : RelationshipEdges[FeatureName]) {
      auto R = std::make_unique<Relationship>(Pair.first);
      Features[FeatureName]->addEdge(R.get());
      R->setParent(Features[FeatureName].get());
      for (const auto &Child : Pair.second) {
        if (Children[FeatureName].count(Child) == 0) {
          llvm::errs() << "error: Related node \'" << Child
                       << "\' is not child of \'" << FeatureName << "\'.\n";
          return false;
        }
        Skip.insert(Child);
        R->addEdge(Features[Child].get());
        Features[Child]->setParent(R.get());
      }
      Relationships.push_back(std::move(R));
    }
  }
  for (const auto &Child : Children[FeatureName]) {
    if (Skip.count(Child) > 0) {
      continue;
    }
    Features[FeatureName]->addEdge(Features[Child].get());
    Features[Child]->setParent(Features[FeatureName].get());
  }
  return true;
}

FeatureModelBuilder *FeatureModelBuilder::setRoot(const std::string &RootName) {
  assert(this->Root == nullptr && "Root already set.");

  if (Features.find(RootName) == Features.end()) {
    makeFeature<BinaryFeature>(RootName, false);
  }
  this->Root = Features[RootName].get();

  for (const auto &FeatureName : Features.keys()) {
    if (FeatureName != RootName && Parents.find(FeatureName) == Parents.end()) {
      Children[RootName].insert(std::string(FeatureName));
      Parents[FeatureName] = RootName;
    }
  }
  return this;
}

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildFeatureModel() {
  if (!Root) {
    setRoot();
  }
  assert(Root && "Root not set.");
  std::set<std::string> Visited;

  if (!buildConstraints() ||
      !buildTree(std::string(Root->getName()), Visited)) {
    return nullptr;
  }
  return std::make_unique<FeatureModel>(Name, Path, Commit, std::move(Features),
                                        std::move(Constraints),
                                        std::move(Relationships), Root);
}

} // namespace vara::feature

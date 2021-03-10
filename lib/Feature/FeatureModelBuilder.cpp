#include "vara/Feature/FeatureModelBuilder.h"

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                        FeatureModelBuilder
//===----------------------------------------------------------------------===//

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
      const std::string FName{Frontier.back()};
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

bool FeatureModelBuilder::buildTree(Feature &F,
                                    std::set<std::string> &Visited) {
  if (find(Visited.begin(), Visited.end(), F.getName()) != Visited.end()) {
    llvm::errs() << "error: Cycle or duplicate edge in \'" << F.getName()
                 << "\'.\n";
    return false;
  }
  Visited.insert(F.getName().str());

  for (const auto &Child : Children[F.getName()]) {
    if (Parents[Child] != F.getName()) {
      llvm::errs() << "error: Parent of \'" << Child << "\' does not match \'"
                   << F.getName() << "\'.\n";
      return false;
    }
    if (auto *C = getFeature(Child); C) {
      if (!buildTree(*C, Visited)) {
        return false;
      }
    } else {
      llvm::errs() << "error: Missing feature \'\'" << F.getName() << "\'.\n";
    }
  }

  llvm::SmallSet<std::string, 3> Skip;
  if (RelationshipEdges.find(F.getName()) != RelationshipEdges.end()) {
    for (const auto &Pair : RelationshipEdges[F.getName()]) {
      auto R = std::make_unique<Relationship>(Pair.first);
      Features[F.getName()]->addEdge(R.get());
      R->setParent(Features[F.getName()].get());
      for (const auto &Child : Pair.second) {
        if (Children[F.getName()].count(Child) == 0) {
          llvm::errs() << "error: Related node \'" << Child
                       << "\' is not child of \'" << F.getName() << "\'.\n";
          return false;
        }
        Skip.insert(Child);
        R->addEdge(Features[Child].get());
        Features[Child]->setParent(R.get());
      }
      Relationships.push_back(std::move(R));
    }
  }

  for (const auto &Child : Children[F.getName()]) {
    if (Skip.count(Child) > 0) {
      continue;
    }
    Features[F.getName()]->addEdge(Features[Child].get());
    Features[Child]->setParent(Features[F.getName()].get());
  }

  return true;
}

bool FeatureModelBuilder::buildRoot() {
  for (auto *Child : Root->children()) {
    if (auto *C = llvm::dyn_cast<Feature>(Child);
        C && Parents[C->getName()].empty()) {
      Children[Root->getName()].insert(C->getName().str());
      Parents[C->getName()] = Root->getName();
    }
  }
  return true;
}

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildFeatureModel() {
  std::set<std::string> Visited;
  if (!buildRoot() || !buildConstraints() || !buildTree(*Root, Visited)) {
    return nullptr;
  }
  return std::make_unique<FeatureModel>(Name, Path, Commit, std::move(Features),
                                        std::move(Constraints),
                                        std::move(Relationships), Root);
}

} // namespace vara::feature

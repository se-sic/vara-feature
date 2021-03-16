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

bool FeatureModelBuilder::detectXMLAlternatives() {
  for (auto *Node : FM->features()) {
    std::vector<FeatureTreeNode *> Frontier;
    std::copy_if(Node->begin(), Node->end(), std::back_inserter(Frontier),
                 [](auto *N) { return llvm::isa<Feature>(N); });
    while (!Frontier.empty()) {
      auto *B = Frontier.back();
      Frontier.pop_back();
      if (auto *F = llvm::dyn_cast<Feature>(B); F && !F->isOptional()) {
        llvm::SmallSet<Feature *, 3> Xor;
        Xor.insert(F);
        for (auto *FF : Frontier) {
          if (auto *E = llvm::dyn_cast<Feature>(FF); E && !E->isOptional()) {
            if (std::all_of(Xor.begin(), Xor.end(), [E](const auto *F) {
                  return isSimpleMutex(F, E);
                })) {
              Xor.insert(E);
            }
          }
        }
        if (Xor.size() > 1) {
          for (auto *E : Xor) {
            Frontier.erase(std::remove(Frontier.begin(), Frontier.end(), E),
                           Frontier.end());
            // TODO
            //            for (auto *R : cleanUpMutualExclusiveConstraints(E,
            //            Xor)) {
            //              E->removeConstraintNonPreserve(R);
            //            }
          }
          // TODO
          //          Special.addRelationship(
          //              Relationship::RelationshipKind::RK_ALTERNATIVE, F,
          //              std::set(Xor.begin(), Xor.end()));
        }
      }
    }
  }
  return true;
}

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildFeatureModel() {
  if (!FM->getRoot()) {
    auto T = FeatureModelModifyTransaction::openTransaction(*FM);
    T.setRoot(std::make_unique<RootFeature>("root"));
    if (!T.commit()) {
      return nullptr;
    }
  }
  return Features.commit() && Transactions.commit() &&
                 PostTransactions.commit() && detectXMLAlternatives() &&
                 Special.commit()
             ? std::move(FM)
             : nullptr;
}

} // namespace vara::feature

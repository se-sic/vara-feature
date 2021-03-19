#include "vara/Feature/FeatureModelBuilder.h"

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                        FeatureModelBuilder
//===----------------------------------------------------------------------===//

/// Decide whether feature A excludes B. Beware that this method only detects
/// very simple trees with binary excludes.
bool detectExclude(const Feature *A, const Feature *B) {
  return std::any_of(
      A->excludes().begin(), A->excludes().end(), [A, B](const auto *E) {
        if (const auto *LHS =
                llvm::dyn_cast<PrimaryFeatureConstraint>(E->getLeftOperand())) {
          if (const auto *RHS = llvm::dyn_cast<PrimaryFeatureConstraint>(
                  E->getRightOperand())) {
            return (LHS->getFeature() &&
                    LHS->getFeature()->getName() == A->getName() &&
                    RHS->getFeature() &&
                    RHS->getFeature()->getName() == B->getName());
          }
        }
        return false;
      });
}

bool FeatureModelBuilder::detectXMLAlternatives() {
  for (auto *F : FM->features()) {
    auto Children = F->getChildren<Feature>();
    if (Children.size() > 1 &&
        std::all_of(Children.begin(), Children.end(), [Children](auto *F) {
          return !F->isOptional() &&
                 std::all_of(Children.begin(), Children.end(), [F](auto *C) {
                   return F == C ||
                          (detectExclude(F, C) && detectExclude(C, F));
                 });
        })) {
      Special.addRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE,
                              F);
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

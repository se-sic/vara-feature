#include "vara/Solver/SolverFactory.h"

namespace vara::solver {

std::unique_ptr<Solver>
SolverFactory::initializeZ3Solver(const feature::FeatureModel &Model) {
  std::unique_ptr<Z3Solver> S = Z3Solver::create();
  // Check which feature is in an alternative group and save it for 'addFeature'
  std::vector<string> V;
  for (const auto &R : Model.relationships()) {
    for (const auto &Child : R->children()) {
      const auto *ChildFeature = (const feature::Feature *)Child;
      V.insert(V.begin(), ChildFeature->getName().str());
    }
  }

  // Iterate over all features first
  for (auto *F : Model.features()) {
    S->addFeature(*F, std::find(V.begin(), V.end(), F->getName().str()) != V.end());
  }

  // Iterate over all constraints
  for (const auto &C : Model.constraints()) {
    S->addConstraint(*C);
  }

  // Iterate over all relationships
  for (const auto &R : Model.relationships()) {
    S->addRelationship(*R);
  }
  return S;
}

} // namespace vara::solver
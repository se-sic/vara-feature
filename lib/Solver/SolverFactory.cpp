#include "vara/Solver/SolverFactory.h"

namespace vara::solver {

std::unique_ptr<Solver>
SolverFactory::initializeZ3Solver(const feature::FeatureModel &Model) {
  std::unique_ptr<Z3Solver> S = Z3Solver::create();
  // Iterate over all features first
  for (auto *F : Model.features()) {
    S->addFeature(*F);
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
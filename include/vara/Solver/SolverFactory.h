#ifndef VARA_FEATURE_SOLVERFACTORY_H
#define VARA_FEATURE_SOLVERFACTORY_H

#include "vara/Feature/FeatureModel.h"
#include "vara/Solver/Solver.h"

namespace vara::solver {

class SolverFactory {
public:
  // TODO: Add type as parameter? 0 -> Z3, 1 -> ?, ...
  static std::unique_ptr<Solver> initializeZ3Solver(const feature::FeatureModel &Model);

  static std::unique_ptr<Solver> initializeZ3Solver();
};

} // namespace vara::solver

#endif // VARA_FEATURE_SOLVERFACTORY_H

#ifndef VARA_FEATURE_SOLVERFACTORY_H
#define VARA_FEATURE_SOLVERFACTORY_H

#include "vara/Feature/FeatureModel.h"
#include "vara/Solver/Solver.h"

namespace vara::solver {

class SolverFactory {
public:
  // TODO: Solver -> unique_ptr; template für den Typen
  static Solver initializeSolver(const feature::FeatureModel &Model);

  static Solver initializeSolver();
};

} // namespace vara::solver

#endif // VARA_FEATURE_SOLVERFACTORY_H

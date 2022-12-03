#ifndef VARA_FEATURE_SOLVERFACTORY_H
#define VARA_FEATURE_SOLVERFACTORY_H

#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelBuilder.h"
#include "vara/Solver/Solver.h"

namespace vara::solver {

/// The different solver types supported by VaRA
enum SolverType { Z3 };

/// This class constructs a solver instance that optionally initializes the
/// solver using a certain feature model.
class SolverFactory {
public:
  /// This method returns a pointer to an initialized solver. This solver has
  /// already processed the features and constraints of the given feature model.
  /// \param Model the model to use for the initialization of the solver
  /// \param Type the type of solver to use
  /// \return a unique pointer containing the initialized solver
  static std::unique_ptr<Solver>
  initializeSolver(const feature::FeatureModel &Model, const SolverType Type) {
    switch (Type) {
    case Z3:
      return initializeZ3Solver(Model);
    }
  }

  /// This method returns a pointer to an initialized solver.
  /// \param Type the type of solver to use
  /// \return a unique pointer containing the initialized solver
  static std::unique_ptr<Solver> initializeSolver(const SolverType Type) {
    vara::feature::FeatureModelBuilder B;
    std::unique_ptr<const feature::FeatureModel> FM = B.buildFeatureModel();
    return initializeSolver(*FM, Type);
  }

private:
  /// This method returns an initialized Z3 solver.
  /// \param Model the model to use for the initialization of the solver
  /// \return a unique pointer containing the initialized solver
  static std::unique_ptr<Solver>
  initializeZ3Solver(const feature::FeatureModel &Model);
};

} // namespace vara::solver

#endif // VARA_FEATURE_SOLVERFACTORY_H

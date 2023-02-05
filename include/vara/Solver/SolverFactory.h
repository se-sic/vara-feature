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

  /// \returns a unique pointer containing the initialized solver
  [[nodiscard]] static std::unique_ptr<Solver>
  initializeSolver(const feature::FeatureModel &Model, const SolverType Type) {
    std::unique_ptr<Solver> S;
    switch (Type) {
    case Z3:
      S = initializeZ3Solver();
    }
    return applyModelOnSolver(Model, std::move(S));
  }

  /// This method returns a pointer to an initialized solver.

  /// \param Type the type of solver to use

  /// \returns a unique pointer containing the initialized solver
  [[nodiscard]] static std::unique_ptr<Solver>
  initializeSolver(const SolverType Type) {
    vara::feature::FeatureModelBuilder B;
    std::unique_ptr<const feature::FeatureModel> FM = B.buildFeatureModel();
    return initializeSolver(*FM, Type);
  }

private:
  /// This method returns an initialized Z3 solver.
  /// \param Model the model to use for the initialization of the solver
  /// \return a unique pointer containing the initialized solver
  static std::unique_ptr<Solver> initializeZ3Solver();

  /// This method uses the public solver API to apply the feature model on
  /// the solver.

  /// \returns a solver object containing all features and constraints.
  static std::unique_ptr<Solver>
  applyModelOnSolver(const feature::FeatureModel &Model,
                     std::unique_ptr<Solver> S);
};

} // namespace vara::solver

#endif // VARA_FEATURE_SOLVERFACTORY_H

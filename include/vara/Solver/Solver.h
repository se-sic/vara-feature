#ifndef VARA_SOLVER_SOLVER_H_
#define VARA_SOLVER_SOLVER_H_

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"

#include "vara/Feature/Relationship.h"
#include "z3++.h"

namespace vara::solver {

//===----------------------------------------------------------------------===//
//                               Solver Class
//===----------------------------------------------------------------------===//

/// \brief This class represents an interface for solvers.
class Solver {
public:
  explicit Solver() = default;

  virtual ~Solver() = default;

  /// Adds the given feature to the solver. That is, a new variable is created
  /// in the solver and all its constraints are included.
  /// \return a possible error if adding the feature was not successful.
  virtual Result<SolverErrorCode>
  addFeature(const feature::Feature &FeatureToAdd) = 0;

  /// This method adds a boolean feature with the given name. Please be aware
  /// that this method does only include a boolean feature with no constraints.
  /// If this is not intended, use the other methods, instead.
  /// \return a possible error if adding the boolean feature was not successful.
  virtual Result<SolverErrorCode> addFeature(const string &FeatureName) = 0;

  /// Adds a numeric feature with the given name and the given values.
  /// Be aware that this method includes a numeric feature with no constraints
  /// and the specified values. That is also the case if the values of the
  /// feature are 0 and 1.
  /// If this is not intended, use the other methods, instead.
  /// \return a possible error if adding the numeric feature was not successful.
  virtual Result<SolverErrorCode>
  addFeature(const string &FeatureName, const std::vector<int64_t> &Values) = 0;

  /// This method removes the given feature and all its constraints the feature
  /// is included in.
  /// \return a possible error if removing the feature was not successful.
  virtual Result<SolverErrorCode>
  removeFeature(feature::Feature &FeatureToRemove) = 0;

  /// Processes the given relationship. This can be either an ALTERNATIVE group
  /// or an OR group.
  /// \param R the relationship to add.
  /// \return a possible error if adding the relationship could not be done.
  virtual Result<SolverErrorCode>
  addRelationship(const feature::Relationship &R) = 0;

  /// Adds the given constraint to the solver. In the case that the constraint
  /// includes features that were not included yet, a error is returned.
  /// So, make sure that all features are already added.
  /// \return a possible error if adding the constraint could not be done.
  virtual Result<SolverErrorCode>
  addConstraint(feature::Constraint &ConstraintToAdd) = 0;

  /// Removes the given constraint from the solver.
  /// If the constraint is not found, an error is returned.
  /// \return a possible error if removing the constraint could not be done.
  virtual Result<SolverErrorCode>
  removeConstraint(feature::Constraint &ConstraintToRemove) = 0;

  /// Returns \c true if the current constraint system (i.e., its features and
  /// its constraints) has valid configurations.
  /// \return an error if, for instance, not all constraints could be parsed yet
  /// because of missing variables. Otherwise, it contains a boolean whether the
  /// current constraint system is solvable (\c true) or not (\c false).
  virtual Result<SolverErrorCode, std::unique_ptr<bool>>
  hasValidConfigurations() = 0;

  /// Returns the number of valid configurations of the current constraint
  /// system (i.e., its features and its constraints). In principle, this is a
  /// #SAT call (i.e., enumerating all configurations).
  /// \return an error if the number of valid configurations can not be retried.
  /// This can be the case if there are still constraints left that were not
  /// included into the solver because of missing variables.
  virtual Result<SolverErrorCode, std::unique_ptr<uint64_t>>
  getNumberValidConfigurations() = 0;

  /// Returns the current configuration.
  /// \return the current configuration found by the solver an error code in
  /// case of error.
  virtual Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getCurrentConfiguration() = 0;

  /// This method returns the next configuration or an error in case of an
  /// error. \return the next configuration or an error (e.g., if it is
  /// unsatisfiable).
  virtual Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() = 0;

  /// This method resets the configuration iterator.
  /// \return an error if resetting the configuration iterator should not be
  /// successful.
  virtual Result<SolverErrorCode> resetConfigurationIterator() = 0;

  /// Returns all valid configurations. In comparison to \c
  /// getNumberValidConfigurations, this method returns the configurations
  /// instead of a number of configurations.
  /// \return an error if an error occurs while retrieving the configurations.
  /// Otherwise, it will return the configurations.
  virtual Result<SolverErrorCode, std::unique_ptr<std::vector<std::unique_ptr<
                                      vara::feature::Configuration>>>>
  getAllValidConfigurations() = 0;
};

//===----------------------------------------------------------------------===//
//                               Z3Solver Class
//===----------------------------------------------------------------------===//

/// \brief The Z3 solver implementation for handling constraints
class Z3Solver : public Solver {
  explicit Z3Solver(z3::config &Cfg) : Context(Cfg) {
    Solver = std::make_unique<z3::solver>(Context);
  }

public:
  static Z3Solver create() {
    // The configuration of z3
    z3::config Cfg;
    Cfg.set("model", "true");
    Cfg.set("model-validate", "true");
    return Z3Solver(Cfg);
  }

  Result<SolverErrorCode>
  addFeature(const feature::Feature &FeatureToAdd) override;

  Result<SolverErrorCode> addFeature(const string &FeatureName) override;

  Result<SolverErrorCode>
  addFeature(const string &FeatureName,
             const std::vector<int64_t> &Values) override;

  Result<SolverErrorCode>
  removeFeature(feature::Feature &FeatureToRemove) override;

  Result<SolverErrorCode>
  addRelationship(const feature::Relationship &R) override;

  Result<SolverErrorCode>
  addConstraint(feature::Constraint &ConstraintToAdd) override;

  Result<SolverErrorCode>
  removeConstraint(feature::Constraint &ConstraintToRemove) override;

  Result<SolverErrorCode, std::unique_ptr<bool>>
  hasValidConfigurations() override;

  Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getCurrentConfiguration() override;

  Result<SolverErrorCode, std::unique_ptr<uint64_t>>
  getNumberValidConfigurations() override;

  Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() override;

  Result<SolverErrorCode> resetConfigurationIterator() override;

  Result<SolverErrorCode, std::unique_ptr<std::vector<
                              std::unique_ptr<vara::feature::Configuration>>>>
  getAllValidConfigurations() override;

private:
  // The SolverConstraintVisitor is a friend class to access the solver and the
  // context.
  friend class SolverConstraintVisitor;

  /// Exclude the current configuration by adding it as a constraint
  /// \return an error code in case of error.
  Result<SolverErrorCode> excludeCurrentConfiguration();

  /// Processes the constraints of the binary feature.
  /// \return an error code in case of error.
  Result<SolverErrorCode>
  setBinaryFeatureConstraints(const feature::BinaryFeature &Feature);

  /// This map contains the original feature names as key and maps it to the
  /// Z3 value.
  llvm::StringMap<std::unique_ptr<z3::expr>> OptionToVariableMapping;

  /// This vector contains the constraints that remain unprocessed because of
  /// features/variables that are not included yet into the solver.
  llvm::SmallVector<std::unique_ptr<feature::Constraint>, 10>
      UnprocessedConstraints;

  /// The context of Z3 needed to initialize variables.
  z3::context Context;

  /// The instance of the Z3 solver needed for caching the constraints and
  /// variables.
  std::unique_ptr<z3::solver> Solver;
};

/// \brief This class is a visitor to convert the constraints from the
/// feature model into constraints for Z3.
class SolverConstraintVisitor : public vara::feature::ConstraintVisitor {
public:
  /// Constructs a new solver constraint visitor by using the reference to
  /// the solver. This reference is needed to retrieve the context and reuse
  /// it in the visitor.
  /// \param S The Z3Solver instance that uses this visitor.
  SolverConstraintVisitor(Z3Solver *S)
      : Z3ConstraintExpression(S->Context), S(S){};

  /// This method adds the constraint after visiting the constraint and
  /// constructing it while visiting.
  /// \param C The constraint to visit
  /// \return \c true if adding the constraint was successfull; \c false
  /// otherwise.
  bool addConstraint(vara::feature::Constraint *C);

  /// Visits the binary constraint. Thereby, it constructs the z3 constraint
  /// by first executing the first part of the binary operator and afterwards
  /// the second (right) part.
  /// \param C the binary constraint to be converted
  /// \return \c true if adding the constraint was successfull; \c false
  /// otherwise.
  bool visit(vara::feature::BinaryConstraint *C) override;

  /// Visits the unary constraints. Thereby, it visits the operand and creates
  /// a new z3 constraint by preceeding it with the according expression.
  /// \param C the unary constraint to visit
  /// \return \c true if adding the constraint was successfull; \c false
  /// otherwise.
  bool visit(vara::feature::UnaryConstraint *C) override;

  /// Visits the feature in the constraint. This method creates the according
  /// z3 constant.
  /// \param C
  /// \return \c true if adding the constraint was successfull; \c false
  /// otherwise.
  bool visit(vara::feature::PrimaryFeatureConstraint *C) override;

private:
  /// The z3 constraint will be adjusted while visiting the given constraint
  z3::expr Z3ConstraintExpression;

  /// The reference to the solver, which is basically needed for the context.
  /// Note that the context can not be copied or references, which is why we
  /// do not hold a direct reference to the context here.
  Z3Solver *S;
};

} // namespace vara::solver

#endif // VARA_SOLVER_SOLVER_H_

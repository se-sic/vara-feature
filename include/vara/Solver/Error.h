#ifndef VARA_SOLVER_ERROR_H_
#define VARA_SOLVER_ERROR_H_

#include "vara/Utils/Result.h"

namespace vara {
namespace solver {

/// Represents different error codes on which the solver could abort on.
enum SolverErrorCode {
  NOT_IMPLEMENTED,
  NOT_SUPPORTED,
  UNSAT,
  ALREADY_PRESENT,
  NOT_ALL_CONSTRAINTS_PROCESSED,
  PARENT_NOT_PRESENT,
};

} // namespace solver

template <>
class Error<vara::solver::SolverErrorCode> {
public:
  Error(vara::solver::SolverErrorCode E) : E(E) {}

  vara::solver::SolverErrorCode operator*() { return E; }

  vara::solver::SolverErrorCode extractError() { return E; }

  operator bool() const { return false; }

  friend llvm::raw_ostream &
  operator<<(llvm::raw_ostream &OS,
             const Error<vara::solver::SolverErrorCode> &Error) {
    switch (Error.E) {
    case vara::solver::NOT_IMPLEMENTED:
      OS << "This method is not implemented yet.";
      break;
    case vara::solver::NOT_SUPPORTED:
      OS << "This method is not supported.";
      break;
    case vara::solver::UNSAT:
      OS << "The current model is unsatisfiable.";
      break;
    case vara::solver::ALREADY_PRESENT:
      OS << "Variable is already present.";
      break;
    case vara::solver::NOT_ALL_CONSTRAINTS_PROCESSED:
      OS << "Not all constraints have been processed yet; are there variables "
            "missing?";
      break;
    case vara::solver::PARENT_NOT_PRESENT:
      OS << "Parent feature of a feature is not present.";
      break;
    }
    return OS;
  }

private:
  vara::solver::SolverErrorCode E;
};

} // namespace vara

#endif // VARA_SOLVER_ERROR_H_

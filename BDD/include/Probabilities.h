#ifndef OXIDD_CAPI_PROBABILITIES_H
#define OXIDD_CAPI_PROBABILITIES_H

#include "oxidd/bdd.hpp"
#include "BDDFactory.h"
#include "oxidd/util.hpp"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"
#include <utility>

using vara::Result;
using vara::solver::SolverErrorCode;

namespace bdd::sample {

    Result<vara::solver::SolverErrorCode>getPr(
        const oxidd::bdd_manager &Manager,
        oxidd::bdd_function &Node,
        std::map<oxidd::bdd_function, std::pair<double, double>> *SatMap,
        BDDFactory &Factory
    );
    
} // namespace bdd::sample

#endif // OXIDD_CAPI_PROBABILITIES_H
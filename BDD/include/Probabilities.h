#ifndef OXIDD_CAPI_PROBABILITIES_H
#define OXIDD_CAPI_PROBABILITIES_H

#include "oxidd/bdd.hpp"
#include "BDDFactory.h"
#include "oxidd/util.hpp"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"

using vara::Result;
using vara::solver::SolverErrorCode;

namespace bdd::sample {

    Result<vara::solver::SolverErrorCode>getPr(
        oxidd::bdd_manager *Manager,
        oxidd::bdd_function *Node,
        oxidd::var_no_t Id,
        BDDFactory::BDDFeat *Feat,
        BDDFactory *Factory
    );
    
} // namespace bdd::sample

#endif // OXIDD_CAPI_PROBABILITIES_H
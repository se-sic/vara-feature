#ifndef OXIDD_CAPI_PROBABILITIES_H
#define OXIDD_CAPI_PROBABILITIES_H

#include "oxidd/capi.h"
#include <string>
#include <unordered_map>
#include "BDDFactory.h"
#include "llvm/Support/ErrorHandling.h"
#include "vara/Utils/Result.h"
#include "vara/Solver/Error.h"

using vara::Result;
using vara::solver::SolverErrorCode;
using oxidd::capi::BDDFactory;

namespace oxidd::capi {

    Result<vara::solver::SolverErrorCode>getPr(
        oxidd_bdd_manager_t* manager,
        oxidd_bdd_t* node,
        oxidd_var_no_t id,
        oxidd::capi::BDDFactory::BDDFeat* feat,
        oxidd::capi::BDDFactory& factory
    );
    
}

#endif // OXIDD_CAPI_PROBABILITIES_H
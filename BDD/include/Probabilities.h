#ifndef OXIDD_CAPI_PROBABILITIES_H
#define OXIDD_CAPI_PROBABILITIES_H

#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/Feature.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "BDDFactory.h"
#include "llvm/Support/ErrorHandling.h"
#include "vara/Utils/Result.h"
#include "vara/Solver/Error.h"
#include "Constraints.h"

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Feature;
using std::unordered_map;
using std::string;
using std::vector;
using std::pair;

namespace oxidd::capi {

    vara::Result<vara::solver::SolverErrorCode>getPr(
        oxidd_bdd_manager_t* manager,
        oxidd_bdd_t* node,
        oxidd::capi::BDDFactory::BDDFeat* feat,
        size_t nodeCount,
        oxidd_bdd_t *oneTerminal,
        oxidd_bdd_t *zeroTerminal,
        BDDFactory& factory
    );
    
}

#endif // OXIDD_CAPI_PROBABILITIES_H
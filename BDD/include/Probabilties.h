#ifndef OXIDD_CAPI_PROBABILITIES_H
#define OXIDD_CAPI_PROBABILITIES_H

#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/Feature.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "BDD/include/BDDFactory.h"
#include "llvm/Support/ErrorHandling.h"
#include "vara/Utils/Result.h"
#include "vara/Solver/Error.h"
#include "BDD/include/Constraints.h"

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Feature;
using oxidd::capi::BDDFactory;
using oxidd::capi::BDDFactory::BDDFeat;
using std::unordered_map;
using std::string;
using std::vector;
using std::pair;

namespace oxidd::capi {

    Result<SolverErrorCode>getPr(
        oxidd_bdd_manager_t* manager,
        oxidd_bdd_t* node,
        BDDFactory::BDDFeat* feat,
        size_t nodeCount,
        oxidd_bdd_t *oneTerminal,
        oxidd_bdd_t *zeroTerminal,
        unordered_map<std::string, BDDFactory::BDDFeat>* varMap
    )
    
}

#endif OXIDD_CAPI_PROBABILITIES_H
#ifndef OXIDD_BDD_FEATS
#define OXIDD_BDD_FEATS

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

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Feature;
using oxidd::capi::BDDFactory;

namespace oxidd::capi {
    Result<SolverErrorCode>FeatureToBdd(
        const oxidd_bdd_manager_t* mgr,
        const bool isInXOR,
        const Feature& feature,
        std::unordered_map<oxidd_var_no_t, BDDFactory::BDDFeat>* varMap,
        oxidd_bdd_t* finalBdd
    );

    Result<SolverErrorCode> addFeatureToBdd(
        const string featureName,
        unordered_map<oxidd_var_no_t, BDDFactory::BDDFeat>* varMap,
        oxidd_var_no_t id,
        oxidd_bdd_manager_t manager
    );

    Result<SolverErrorCode> addBinaryConstraints(
        oxidd_var_no_t parentId,
        oxidd_var_no_t id,
        const bool isInXOR,
        const bool isOpt,
        unordered_map<oxidd_var_no_t, BDDFactory::BDDFeat>* varMap,
        oxidd_bdd_t* finalBdd
    );

} // namespace oxidd::capi

#endif // OXIDD_BDD_FEATS
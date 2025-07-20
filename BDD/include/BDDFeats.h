#ifndef OXIDD_BDD_FEATS
#define OXIDD_BDD_FEATS

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
using std::unordered_map;
using std::string;
using std::vector;
using std::pair;

namespace oxidd::capi {
    Result<SolverErrorCode>FeatureToBdd(
        const oxidd_bdd_manager_t manager,
        const bool isInXOR,
        const Feature& feature,
        unordered_map<string, BDDFactory::BDDFeat> *varMap,
        unordered_map<string, oxidd_bdd_t> *binaryVarMap,
        unordered_map<string, vector<pair<string,oxidd_bdd_t>>> *numericVarMap,
        oxidd_bdd_t *finalBDD);

} // namespace oxidd::capi

#endif // OXIDD_BDD_FEATS
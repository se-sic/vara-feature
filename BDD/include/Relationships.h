
#ifndef OXIDD_RELATIONSHIPS
#define OXIDD_RELATIONSHIPS

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
#include "vara/Feature/Relationship.h"

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Relationship;
using vara::feature::Feature;
using oxidd::capi::BDDFactory;
using std::unordered_map;
using std::string;
using std::vector;
using std::pair;

namespace oxidd::capi {
    Result<SolverErrorCode> RelationshipToBdd(
        const oxidd_bdd_manager_t manager,
        Relationship *relationship,
        unordered_map<string, BDDFactory::BDDFeat> *varMap,
        oxidd_bdd_t *finalBDD
    );
} // namespace oxidd::capi

#endif // OXIDD_RELATIONSHIPS

#ifndef OXIDD_BDD_FEATS
#define OXIDD_BDD_FEATS

#include "BDDFactory.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include "vara/Feature/Feature.h"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"
#include <unordered_map>

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Feature;
using bdd::sample::BDDFactory;

namespace bdd::sample {
    Result<SolverErrorCode>featureToBdd(
        const oxidd::bdd_manager *Mgr,
        bool IsInXor,
        const Feature &Feature,
        std::unordered_map<oxidd::level_no_t, BDDFactory::BDDFeat> *VarMap,
        oxidd::bdd_function *FinalBdd
    );

} // namespace bdd::sample

#endif // OXIDD_BDD_FEATS
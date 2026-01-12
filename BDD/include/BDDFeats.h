#ifndef OXIDD_BDD_FEATS
#define OXIDD_BDD_FEATS

#include "BDDFactory.h"
#include "oxidd/bdd.hpp"
#include "vara/Feature/Feature.h"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Feature;
using bdd::sample::BDDFactory;

namespace bdd::sample {
    Result<SolverErrorCode>featureToBdd(
        oxidd::bdd_manager &Mgr,
        bool IsInXor,
        const Feature &Feature,
        oxidd::bdd_function &FinalBdd
    );

} // namespace bdd::sample

#endif // OXIDD_BDD_FEATS
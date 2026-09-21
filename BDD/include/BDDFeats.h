#ifndef OXIDD_BDD_FEATS
#define OXIDD_BDD_FEATS

#include "vara/Feature/Feature.h"
#include "BDDFactory.h"
#include "oxidd/bdd.hpp"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"

namespace bdd::sample {

    /// \brief Encodes a single feature from a feature model into \p FinalBdd as a boolean constraint 
    ///
    /// The feature is encoded according to Z3 rules. This means that the child implies the parent at any times
    /// while the parent implies the child only if the feature is mandatory and not in an XOR group.
    // Root features are also forced to be always true.
    ///
    /// \param Mgr The Bdd manager: It has to have a variable for the feature to be added 
    /// \param IsInXor A flag that determines whether the feature is in an XOR (alternative) group.
    /// \param Feature The feature to be encoded
    /// \param FinalBdd The target BDD to which the boolean constraint will be AND-ed
    ///
    /// \return A error code, specifically 
    ///         - vara::Ok::void() if the AND-ing was successful
    ///         - ILLEGAL_STATE if the feature is not registered in the BDD manager
    ///         - PARENT_NOT_PRESENT if the parent feature exists but its variable is not registered in the BDD manager
    ///         - NOT_SUPPORTED if the feature is not a binary one (e.g. numeric feature)
    vara::Result<vara::solver::SolverErrorCode> featureToBdd(
        oxidd::bdd_manager &Mgr,
        bool IsInXor,
        const vara::feature::Feature &Feature,
        oxidd::bdd_function &FinalBdd
    );

} // namespace bdd::sample

#endif // OXIDD_BDD_FEATS
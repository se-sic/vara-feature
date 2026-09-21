#ifndef BDD_SAMPLE_INTERACTION_COVERAGE_H
#define BDD_SAMPLE_INTERACTION_COVERAGE_H

#include "EnumerateInteractions.h"

#include <vector>

namespace bdd::sample {

    /// \brief Checks whether the \p Configuration covers the \p Interaction
    ///
    /// An interaction is a set of (variable, value) pairs. The configuration therefore matches the interaction if for every pair, the configuration's
    /// value for that variable matches the value in the interaction.
    ///
    /// \param Configuration Binary assignment of all variables of the feature model
    /// \param Interaction Set of (variable, value) pairs to check coverage againt. Every entry in Variables is a valid index in Configuration.
    ///
    /// \return True if configuration matches all (variable, value) pairs in the interaction, false otherwise
    bool CoverageCheck(const std::vector<bool> &Configuration, const bdd::sample::Interaction &Interaction); //NOLINT

} // namespace bdd::sample

#endif // BDD_SAMPLE_INTERACTION_COVERAGE_H
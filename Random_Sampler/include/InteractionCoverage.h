#ifndef BDD_SAMPLE_INTERACTION_COVERAGE_H
#define BDD_SAMPLE_INTERACTION_COVERAGE_H

#include "EnumerateInteractions.h"
#include <vector>

namespace bdd::sample {

    bool CoverageCheck(const std::vector<bool> &Configuration, const bdd::sample::Interaction &Interaction); //NOLINT

} // namespace bdd::sample

#endif // BDD_SAMPLE_INTERACTION_COVERAGE_H
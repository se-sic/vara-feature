#ifndef BDD_SAMPLE_T_WISE_SAMPLER_H
#define BDD_SAMPLE_T_WISE_SAMPLER_H

#include "EnumerateInteractions.h"
#include <vector>

namespace bdd::sample {

   std::vector<std::vector<bool>> TSample(const std::vector<std::vector<bool>> &ValidConfigs, const std::vector<bdd::sample::Interaction> &Interactions); //NOLINT

} // namespace bdd::sample

#endif // BDD_SAMPLE_T_WISE_SAMPLER_H
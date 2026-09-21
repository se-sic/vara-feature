#ifndef BDD_SAMPLE_T_WISE_SAMPLER_H
#define BDD_SAMPLE_T_WISE_SAMPLER_H

#include "EnumerateInteractions.h"

#include <vector>

namespace bdd::sample {

   /// \brief Greedy t-wise sampler
   ///
   /// Iteratively picks the configuration from \p ValidConfig covering the largest amount of yet-uncovered interactions from \p Interactions, until no more 
   /// configuration can cover any remaining interactions. Interactions are encoded as a bit-packed matrix (one uint64 word per 64 interactions). 
   ///
   /// \param ValidConfigs All valid configuratiions of the feature model
   /// \param Interactions All valid t-wise interactions to cover
   ///
   /// \return A sample subset of \p ValidConfigs covering the maximum achievable amount of interactions from \p Interactions.
   std::vector<std::vector<bool>> TSample(const std::vector<std::vector<bool>> &ValidConfigs, const std::vector<bdd::sample::Interaction> &Interactions); //NOLINT

} // namespace bdd::sample

#endif // BDD_SAMPLE_T_WISE_SAMPLER_H
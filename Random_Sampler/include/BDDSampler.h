#ifndef BDD_SAMPLER_H
#define BDD_SAMPLER_H

#include "BDDFactory.h"
#include "oxidd/bdd.hpp"
#include <cstdlib> //for RAND_MAX
#include <utility>
#include <vector>

namespace bdd::sample {
  
  /// \brief Returns a uniform random double in [0, 1)
  double random();

  /// \brief Seeds the global RNG with a given seed
  void SeedRng(std::uint32_t Seed); //NOLINT

  /// \brief Samples one uniformly-random valid configuration from the BDD
  ///
  /// Traverse through the BDD from the top \p Root, at each internal node choosing the true or false cofactor
  /// with the probability stored in \p SatMap. Variables that the BDD reduces away are unconstrained and
  /// receive an independent draw, where the probabilty of either true or false is 50%.
  ///
  /// \param Manager The BDD manager providing the variable count
  /// \param Root The BDD to sample from
  /// \param SatMap Precomputed mapping between variables of the feature model and their probabilities of being 
  ///               true during traversal
  ///
  /// \return One valid configuration of a boolean vector of length equal to the number of variables registered
  /// in the \p Manager. 
  std::vector<bool>
  generateConfiguration(const oxidd::bdd_manager &Manager, 
              const oxidd::bdd_function &Root, 
              std::map<oxidd::bdd_function, std::pair<double, double>> *SatMap);
} // namespace bdd::sample

#endif //BDD_SAMPLER_H
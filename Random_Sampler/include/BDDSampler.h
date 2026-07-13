#ifndef BDD_SAMPLER_H
#define BDD_SAMPLER_H

#include "../../BDD/include/BDDFactory.h"
#include "oxidd/bdd.hpp"
#include <cstdlib> //for RAND_MAX
#include <utility>
#include <vector>

namespace bdd::sample {
  
  double random();

  void SeedRng(std::uint32_t Seed); //NOLINT

  std::vector<bool>
  generateConfiguration(const oxidd::bdd_manager &Manager, 
              const oxidd::bdd_function &Root, 
              BDDFactory &Factory,
              std::map<oxidd::bdd_function, std::pair<double, double>> *SatMap);
} // namespace bdd::sample

#endif //BDD_SAMPLER_H
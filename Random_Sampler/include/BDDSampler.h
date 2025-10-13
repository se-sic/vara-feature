#ifndef BDD_SAMPLER_H
#define BDD_SAMPLER_H

#include "../../BDD/include/BDDFactory.h"
#include "oxidd/bdd.hpp"
#include <cstdlib> //for RAND_MAX
#include <unordered_map>

namespace bdd::sample {
  
  double random();

  std::unordered_map<oxidd::var_no_t, bool>
  generateConfiguration(const oxidd::bdd_manager &Manager, 
              const oxidd::bdd_function &Root, 
              BDDFactory &Factory);
} // namespace bdd::sample

#endif //BDD_SAMPLER_H
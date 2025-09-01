#ifndef BDD_SAMPLER_H
#define BDD_SAMPLER_H

#include "../../BDD/include/BDDFactory.h"
#include "oxidd/capi.h"
#include <cstdlib> //for RAND_MAX
#include <unordered_map>

namespace oxidd::capi {
    
    double random();

    bool is_bdd_true(const oxidd_bdd_t& f, oxidd_bdd_manager_t manager);

  std::unordered_map<oxidd_var_no_t, bool>
  generateConfiguration(oxidd_bdd_manager_t manager, 
                        oxidd_bdd_t root, 
                        BDDFactory &factory);
}

#endif //BDD_SAMPLER_H
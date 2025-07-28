#ifndef BDD_SAMPLER_H
#define BDD_SAMPLER_H

#include "../../BDD/include/BDDFactory.h"
#include <unordered_map>
#include <cstdlib>
#include "capi.h"

namespace oxidd::capi {

    std::unordered_map<oxidd_level_no_t , bool> sample;
    std::unordered_map<oxidd_bdd_t, double, BDDHash> pr; 

    struct BDDHash {
        size_t operator()(const oxidd_bdd_t& bdd) const {
            return std::hash<const void*>{}(bdd._p) ^ std::hash<size_t>{}(bdd._i);
        }
    };

    double random() {
        return static_cast<double>(std::rand()) / RAND_MAX;
    };

    bool is_bdd_true(const oxidd_bdd_t& f, oxidd_bdd_manager_t manager) {
        oxidd_bdd_t true_node = oxidd_bdd_true(manager);
        return f._p == true_node._p && f._i == true_node._i;
    };

    std::unordered_map<size_t, bool> generateConfiguration(
        oxidd_bdd_t ROOT,
        unordered_map<std::string, BDDFactory::BDDFeat>* varMap)
}

#endif //BDD_SAMPLER_H
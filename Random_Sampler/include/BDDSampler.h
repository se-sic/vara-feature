#ifndef BDD_SAMPLER_H
#define BDD_SAMPLER_H

#include "BDDFactory.h"
#include <unordered_map>
#include <cstdlib>
#include "oxidd/capi.h"

namespace oxidd::capi {

    struct BDDHash {
        size_t operator()(const oxidd_bdd_t& bdd) const {
            return std::hash<const void*>{}(bdd._p) ^ std::hash<size_t>{}(bdd._i);
        }
    };

    extern std::unordered_map<oxidd_level_no_t , bool> sample;
    extern std::unordered_map<oxidd_bdd_t, double, BDDHash> pr; 

    double random();

    bool is_bdd_true(const oxidd_bdd_t& f, oxidd_bdd_manager_t manager);

    std::unordered_map<oxidd_level_no_t , bool> generateConfiguration(
        oxidd_bdd_t ROOT,
        BDDFactory& factory);
}

#endif //BDD_SAMPLER_H
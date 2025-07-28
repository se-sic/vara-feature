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
    }

    bool is_bdd_true(const oxidd_bdd_t& f, oxidd_bdd_manager_t manager) {
        oxidd_bdd_t true_node = oxidd_bdd_true(manager);
        return f._p == true_node._p && f._i == true_node._i;
    }

    std::unordered_map<size_t, bool> generateConfiguration(
        oxidd_bdd_t ROOT,
        unordered_map<std::string, BDDFactory::BDDFeat>* varMap) {
        oxidd_bdd_t trav = ROOT;
        oxidd_bdd_manager_t manager = oxidd_bdd_containing_manager(ROOT);
        oxidd_bdd_t TRUE = oxidd_bdd_true(manager);

        for (uint32_t i = 0; i < oxidd_bdd_level(ROOT); ++i) {
            sample[i] = (random() < 0.5);
        }

        while (trav._p != nullptr && !is_bdd_true(trav, manager)) {
            uint32_t ind = oxidd_bdd_level(trav);
            oxidd_bdd_pair_t cofactors = oxidd_bdd_cofactors(trav);
            oxidd::capi::BDDFactory::BDDFeat* trav_feat = findFeatureinBDD(&trav, &varMap);

            if (random() < trav_feat->info.probability) {
                trav = cofactors.first;  
                sample[ind] = true;
            } else {
                trav = cofactors.second; 
                sample[ind] = false;
            }

            for (uint32_t i = ind + 1; i < oxidd_bdd_level(ROOT); ++i) {
                sample[i] = (random() < 0.5);
            }
        }

        return sample;
    }
}
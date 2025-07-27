//random number generator
//überall wo index ist oxidd_bdd_t
// BDDFactory.cpp benutzen isRoot und index
//sample is wie ne map. key mit dem index, ob der true oder false ist
//wahrscheinlichkeit is auch ne map machen
// HI = true, LO = false
//3.3 4.1 5.2

#include <unordered_map>
#include <cstdlib>
#include "capi.h"

std::unordered_map<size_t, bool> sample;
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

std::unordered_map<size_t, bool> generateConfiguration(oxidd_bdd_t ROOT) {
    oxidd_bdd_t trav = ROOT;
    oxidd_bdd_manager_t manager = oxidd_bdd_containing_manager(ROOT);
    oxidd_bdd_t TRUE = oxidd_bdd_true(manager);

    for (size_t i = 0; i < ROOT._i; ++i) {
        sample[i] = (random() < 0.5);
    }

    while (trav._p != nullptr && !is_bdd_true(trav, manager)) {
        size_t ind = trav._i;

        oxidd_bdd_pair_t cofactors = oxidd_bdd_cofactors(trav);
        if (random() < pr[trav]) {
            trav = cofactors.first;  
            sample[ind] = true;
        } else {
            trav = cofactors.second; 
            sample[ind] = false;
        }

        for (size_t i = ind + 1; i < trav._i; ++i) {
            sample[i] = (random() < 0.5);
        }
    }

    return sample;
}
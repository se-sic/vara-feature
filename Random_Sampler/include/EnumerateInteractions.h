#ifndef BDD_SAMPLE_ENUMERATE_INTERACTIONS_H
#define BDD_SAMPLE_ENUMERATE_INTERACTIONS_H

#include "oxidd/bdd.hpp"
#include <vector>

namespace bdd::sample {

    oxidd::bdd_function Literal(const oxidd::bdd_manager &Manager, oxidd::var_no_t VarID, bool VarValue); //NOLINT

    bool AreLiteralsSat(const oxidd::bdd_manager &Manager, const oxidd::bdd_function &FinalBDD, const std::vector<oxidd::var_no_t> &Variables, const std::vector<bool> &VarValues); //NOLINT

    struct Interaction {
        std::vector<oxidd::var_no_t> Variables; 
        std::vector<bool> Values;
    };

    std::vector<Interaction> EnumerateInteractions(const oxidd::bdd_manager &Manager, const oxidd::bdd_function &FinalBDD, unsigned T); //NOLINT

} // namespace bdd::sample

#endif // BDD_SAMPLE_ENUMERATE_INTERACTIONS_H
#ifndef BDD_SAMPLE_ENUMERATE_INTERACTIONS_H
#define BDD_SAMPLE_ENUMERATE_INTERACTIONS_H

#include "oxidd/bdd.hpp"

#include <vector>

namespace bdd::sample {

    /// \brief Builds a BDD function for a single literal (variable and its value)
    ///
    /// Due to performance reasons, we do not apply this function anymore, instead, we verify against all valid configurations
    oxidd::bdd_function Literal(const oxidd::bdd_manager &Manager, oxidd::var_no_t VarID, bool VarValue); //NOLINT Invalid case style for function 'Literal'

    /// \brief Check if the given literals is exhibited by any configuration in \p ValidConfigs
    ///
    /// \param Variables Feature indices that make up the literals of the interaction
    /// \param VarValues Values for each variable, parallel to \p Variables
    /// \param ValidConfigs All satisfying configuration of the feature model
    ///
    /// \return True if at least one configuration macthes all literals of the interaction, false otherwise
    bool AreLiteralsSat(const std::vector<oxidd::var_no_t> &Variables,                                   //NOLINT Invalid case style for function 'AreLiteralsSat'
                        const std::vector<bool> &VarValues,
                        const std::vector<std::vector<bool>> &ValidConfigs);

    /// \brief A t-wise interaction: a set of variables with their corresponding binary value
    struct Interaction {
        std::vector<oxidd::var_no_t> Variables; 
        std::vector<bool> Values;
    };

    /// \brief Enuerates all valid t-wise interactions of the feature model
    ///
    /// For every combination of \p T variables and every possible value assignment to them, we check
    /// whetehr at least one configuration in \p ValidConfigs exhibits this assignment.
    ///
    /// \param Manager BDD manager for the variable count
    /// \param T Interaction strength
    /// \param ValidConfigs All satisfying configuration of the feature model
    ///
    /// \return A vector of all valid t-wise interactions
    std::vector<Interaction> EnumerateInteractions(const oxidd::bdd_manager &Manager,                 //NOLINT Invalid case style for function 'EnumerateInteractions'
                                                   unsigned T, 
                                                   const std::vector<std::vector<bool>> &ValidConfigs);

} // namespace bdd::sample

#endif // BDD_SAMPLE_ENUMERATE_INTERACTIONS_H
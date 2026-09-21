#ifndef BDD_SAMPLE_ENUMERATE_CONFIGS_H
#define BDD_SAMPLE_ENUMERATE_CONFIGS_H

#include "oxidd/bdd.hpp"

#include <vector>

namespace bdd::sample {

    /// \brief Enumerates all satisfying variabel assignments of \p Root
    ///
    /// Recursively walks the BDD starting from \p Root an returns one 
    /// boolean vector per satisfying asignment upon reaching a true terminal.
    /// Variables that have not been checked along a specific path are treated
    /// as unconstrained, which means that both possible values are enumerated.
    ///
    /// \param Manager The BDD Manger, providing the total variable count 
    /// \param Root Root BDD function whose satisfying assignments are enumerated
    ///
    /// \return A vector, where each entry is a satisfying assignment of length of the total 
    /// number of variables in the BDD manager.
    std::vector<std::vector<bool>> EnumerateConfigs(const oxidd::bdd_manager &Manager, const oxidd::bdd_function &Root); //NOLINT Invalid case style for function 'EnumerateConfigs'

} // namespace bdd::sample

#endif // BDD_SAMPLE_ENUMERATE_CONFIGS_H
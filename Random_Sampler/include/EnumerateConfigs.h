#ifndef BDD_SAMPLE_ENUMERATE_CONFIGS_H
#define BDD_SAMPLE_ENUMERATE_CONFIGS_H

#include "oxidd/bdd.hpp"
#include <vector>

namespace bdd::sample {

    std::vector<std::vector<bool>> EnumerateConfigs(const oxidd::bdd_manager &Manager, //NOLINT
                                                    const oxidd::bdd_function &Root);

} // namespace bdd::sample

#endif // BDD_SAMPLE_ENUMERATE_CONFIGS_H
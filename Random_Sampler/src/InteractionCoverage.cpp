#include "InteractionCoverage.h"
#include "EnumerateInteractions.h"

#include <cstddef>
#include <vector>

namespace bdd::sample {

    bool CoverageCheck(const std::vector<bool> &Configuration, const bdd::sample::Interaction &Interaction) {
        for (std::size_t I = 0; I < Interaction.Variables.size(); I++) {
            if (Configuration[Interaction.Variables[I]] != Interaction.Values[I]) {
                return false;
            }
        }
        return true;
    }

} //namespace bdd::sample
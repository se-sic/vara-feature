#include "EnumerateInteractions.h"
#include "InteractionCoverage.h"
#include <vector>


namespace bdd::sample {

    std::vector<std::vector<bool>> TSample(const std::vector<std::vector<bool>> &ValidConfigs, const std::vector<bdd::sample::Interaction> &Interactions) { //NOLINT
        std::vector<bool> Uncovered(Interactions.size(), true);
        std::vector<bool> Chosen(ValidConfigs.size(), false);
        std::vector<std::vector<bool>> Sample;

        while(true) {
            std::size_t BestConfig = 0;
            std::size_t MaxCoveredInts = 0;
            bool BetterFound = false;

            for (std::size_t C = 0; C < ValidConfigs.size(); C++) {
                if(Chosen[C]) { continue;}

                std::size_t CoveredInts = 0;
                for (std::size_t I = 0; I < Interactions.size(); I++) {
                    if(Uncovered[I] && bdd::sample::CoverageCheck(ValidConfigs[C], Interactions[I])) {
                        CoveredInts++;
                    }
                }
                if(CoveredInts > MaxCoveredInts){
                    MaxCoveredInts = CoveredInts;
                    BestConfig = C;
                    BetterFound = true;
                }
            }

            if(!BetterFound || (MaxCoveredInts == 0)) {
                break;
            }

            Chosen[BestConfig] = true;
            Sample.push_back(ValidConfigs[BestConfig]);
            for (std::size_t I = 0; I < Interactions.size(); I++) {
                if(Uncovered[I] && bdd::sample::CoverageCheck(ValidConfigs[BestConfig], Interactions[I])) {
                    Uncovered[I] = false;
                }
            }
        }
        return Sample;
    }
} //namespace bdd::sample
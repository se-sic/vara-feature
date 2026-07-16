#include "EnumerateInteractions.h"
#include "InteractionCoverage.h"
#include <bit>
#include <cstddef>
#include <vector>


namespace bdd::sample {

    std::vector<std::vector<bool>> TSample(const std::vector<std::vector<bool>> &ValidConfigs, const std::vector<bdd::sample::Interaction> &Interactions) { //NOLINT
        //std::vector<bool> Uncovered(Interactions.size(), true);
        std::vector<bool> Chosen(ValidConfigs.size(), false);
        std::vector<std::vector<bool>> Sample;
        const std::size_t ValidConfigSize = ValidConfigs.size();
        const std::size_t IntSize = Interactions.size();

        // To increase performane and allow sampling in larger constrained systems with more configurations, we encode interactions as bits
        // Words in this case represent how many uint64_t's are required to store the amount of interaction bits
        const std::size_t Words = (IntSize + 63) / 64;
        std::vector<std::uint64_t> Uncovered(Words, ~std::uint64_t{0});
     
        // We precompute a coverage matrix, that, for each configuration, it checks whether it covers teh current interaction
        std::vector<std::uint64_t> Coverage(ValidConfigSize * Words, 0);
        for (std::size_t Config = 0; Config < ValidConfigSize; ++Config) {
            for (std::size_t Int = 0; Int < IntSize; ++Int) {
                if (bdd::sample::CoverageCheck(ValidConfigs[Config], Interactions[Int])) {
                    Coverage[(Config * Words) + (Int / 64)] |= (std::uint64_t{1} << (Int % 64));
                }
            }
        }

        // If amount of Words needed to be rounded up, the resulting tail needs to be masked to 0 so that it is not considered
        if(IntSize % 64) {
            Uncovered[Words - 1] = (std::uint64_t{1} << (IntSize % 64)) - 1;
        }

        while(true) {
            std::size_t BestConfig = 0;
            std::size_t MaxCoveredInts = 0;
            //bool BetterFound = false;

            for (std::size_t Config = 0; Config < ValidConfigs.size(); Config++) {
                if(Chosen[Config]) { continue;}

                std::size_t CoveredInts = 0;
                for (std::size_t W = 0; W < Words; W++) {
                    CoveredInts += std::popcount(Coverage[(Config * Words) + W] & Uncovered[W]);
                }

                if(CoveredInts > MaxCoveredInts){
                    MaxCoveredInts = CoveredInts;
                    BestConfig = Config;
                    //BetterFound = true;
                }
            }

            if(MaxCoveredInts == 0) {
                break;
            }

            Chosen[BestConfig] = true;
            Sample.push_back(ValidConfigs[BestConfig]);
            for (std::size_t W = 0; W < Words; W++) {
                Uncovered[W] &= ~Coverage[(BestConfig * Words) + W];
            }
        }
        return Sample;
    }
} //namespace bdd::sample
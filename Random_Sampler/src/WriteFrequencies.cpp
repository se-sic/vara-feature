#include "WriteFrequencies.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace bdd::sample {

    void WriteFrequencyCSV(const oxidd::bdd_function &FinalBDD, const std::vector<std::vector<bool>> &Samples) {
        std::map<std::vector<bool>, std::size_t> CountConfig;

        // Validate the generated configuration against the BDD
        for (const auto &Config: Samples) {
            std::vector<std::pair<oxidd::var_no_t, bool>> ValidSamples;
            ValidSamples.reserve(Config.size());
            for(oxidd::var_no_t V = 0; V < Config.size(); ++V) {
                ValidSamples.emplace_back(V, Config[V]);
            }
            if(!FinalBDD.eval(ValidSamples)){
                std::cerr << "Error: Generated an invalid configuration at iteration.\n";
                return;
            }
            ++CountConfig[Config];
        }

        // Write the frequency counts to a CSV file
        std::ofstream Out("Random_Sampler/scripts/Configs.csv");
        if (!Out) {
            std::cerr << "Error: Could not open file for writing" << "\n";
            return;
        }

        Out << "ConfigID,Count\n";
        std::size_t Id = 0;
        for (auto &[Config, Count] : CountConfig) {
            Out << Id++ << "," << Count << '\n';
        }
        Out.close();
    }

} //namespace bdd::sample
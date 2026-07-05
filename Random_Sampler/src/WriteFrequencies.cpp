#include "Plotter.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

namespace bdd::sample {

    void WriteFrequencyCSV(const oxidd::bdd_function &FinalBDD, std::vector<std::vector<bool>> &Sample) { //NOLINT
        //Map to store the frequency of each configuration
        std::map<std::vector<bool>, int> CountConfig;
        // Validate the generated configuration against the BDD
        std::vector<std::pair<oxidd::var_no_t, bool>> ValidSamples; // NOLINT
        for (const auto &Config: Sample) {
            ValidSamples.reserve(Config.size());
            for(oxidd::var_no_t V = 0; V < Config.size(); ++V) {
                ValidSamples.emplace_back(V, Config[V]);
            }
            if(!FinalBDD.eval(ValidSamples)){
                std::cerr << "Generated an invalid configuration at iteration.\n";
                std::abort();
            }
            ++CountConfig[Config];
        }

        // Write the frequency counts to a CSV file
        std::ofstream Out("Random_Sampler/scripts/Configs.csv");
        Out << "ConfigID,Count\n";
        int Id = 0;
        for (auto &[Config, Count] : CountConfig) {
            Out << Id++ << "," << Count << '\n';
        }
        Out.close();
    }
} //namespace bdd::sample
#include "BDDSampler.h"
#include "../../BDD/include/BDDFactory.h"
#include "EnumerateConfigs.h"
#include "EnumerateInteractions.h"
#include "InteractionCoverage.h"
#include "ParseFM.h"
#include "Plotter.h"
#include "TWiseSampler.h"
#include "WriteFrequencies.h"
#include "WriteSamples.h"
#include "oxidd/util.hpp"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelParser.h"
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
extern "C" {
#include <oxidd/bdd.hpp>
}

using std::string;
using std::vector;

/**
    * @brief BDD-based sampler for feature model configurations and interactions (t-wise and uniform random sampling).
    *
    * ----------------------------------------------------------------------------------------------------------------
    * HOW TO RUN 
    * ----------------------------------------------------------------------------------------------------------------
    * 1. Build (from repo root) if not already done:
        * cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DVARA_FEATURE_USE_Z3_SOLVER=ON -B build ninja -C build
    * 2. Run the program with following arguments: 
        * argv[0]: -- <path_to_feature_model.xml>
        * argv[1]: -- Command ("tsizes" or "sample" or "sat_count")
        * argv[2]: -- For "tsizes": <t-value> (1, 2, or 3) | For "sample": <sampling_strategy> ("twise" or "random")
        * argv[3]: -- For "sample" with "twise": <t-value> (1, 2, or 3) | For "sample" with "random": <sample_size>
        * argv[4]: -- For "sample" with "random": <seed_value>
        2.1 From the repo root: 
            * ./build/bin/random_sampler argv
            * Example: ./build/bin/random_sampler Random_Sampler/examples/Polly.xml tsizes 2
*/


int main(int argc, char* argv[]) noexcept(false){ 

    //--------------Parsing--------------//

    if (argc < 3) {
        std::cerr << "Usage: ./random_sampler <feature_model.xml> <t-sizes | samples>\n";
        return 1;
    }

    std::vector<std::string> Args(argv + 1, argv + argc); //NOLINT
    std::string FMPath = Args[0];
    const std::string &Command = Args[1];

    std::string Sys = FMPath;
    if (auto S = Sys.find_last_of("/ \\"); S != std::string::npos) {
        Sys = Sys.substr(S + 1);
    }
    if (auto D = Sys.find_last_of('.'); D != std::string::npos) {
        Sys = Sys.substr(0, D);
    }

    //Parse XML to a BDD-readable format
    std::unique_ptr<vara::feature::FeatureModel> Fd = bdd::sample::ParseXML(FMPath);
    std::cerr << "Feature Model loaded successfully from: " << FMPath <<'\n';

    // Create BDD factory and convert feature model to BDD
    bdd::sample::BDDFactory Factory;
    oxidd::bdd_function FinalBDD = Factory.modelToBdd(*Fd);
    oxidd::bdd_manager Manager = FinalBDD.containing_manager(); 
    std::cerr << "BDD constructed successfully." << '\n';

    //Enumeration of all configurations
    auto ValidCondfigs = bdd::sample::EnumerateConfigs(Manager, FinalBDD);
    double Expected = FinalBDD.sat_count_double(Manager.num_vars());
    std::cerr << "Valid Configs: " << ValidCondfigs.size() << " (expected " << Expected << ")\n";
    if (ValidCondfigs.size() != static_cast<std::size_t>(Expected)) {
        std::cerr << "Enumeration count mismatch\n";
    }
    std::cerr << "Enumerated all valid configurations successfully." << '\n';

    // Command: tsizes --> Enumeration of all t-many-interactions + RQ1 sample sizes
    if (Command == "tsizes") {
        unsigned T = 0;
        try {
            T = static_cast<unsigned>(std::stoul(Args[2]));
        } catch (const std::invalid_argument &E) {
            std::cerr << "Invalid value for t: " << Args[2] << '\n';
            return 1;
        }   

        if (T < 1 || T > 3) {
            std::cerr << "Value of t must be between 1 and 3 (inclusive). Provided: " << T << '\n';
            return 1;
        }
        auto Interactions = bdd::sample::EnumerateInteractions(Manager, FinalBDD, T, ValidCondfigs);
        auto S = bdd::sample::TSample(ValidCondfigs, Interactions);

        // Validate the generated configurations against the BDD
        for (const auto &Config : S) {
        std::vector<std::pair<oxidd::var_no_t, bool>> ValidTSamples;
            ValidTSamples.reserve(Config.size());
            for(oxidd::var_no_t V = 0; V < Config.size(); ++V) {
                ValidTSamples.emplace_back(V, Config[V]);
            }
            if(!FinalBDD.eval(ValidTSamples)){
                std::cerr << "Generated an invalid configuration at iteration. This should not happen.\n";
                return 1;
            }
        }

        // Validate the configurations against the coverage of each interaction 
        for (const auto &I: Interactions) {
            bool CoveredInts = false;
            for (const auto &Config : S) {
                if(bdd::sample::CoverageCheck(Config, I)) {
                    CoveredInts = true; break;
                }
            }
            if(!CoveredInts) {
                std::cerr << "Sample did not cover all interactions" << "\n";
            }
        }

        std::cout << S.size() << "\n";
        std::string TPath = (T == 1 ? "T1" : (T == 2 ? "T2" : "T3")); //NOLINT
        std::string TFilePath = "bindings/python/Interplay_ML/Samples/" + TPath + "/" + Sys + "_TWiseSample.csv";
        std::filesystem::create_directories(std::filesystem::path(TFilePath).parent_path());
        bdd::sample::WriteSampleCSV(Manager, S, TFilePath);
        return 0;
    }

    // Command: sample --> Generate t-wise sample and uniform random samples
    if (Command == "sample") {
        std::string &Strat = Args[2];
        
        if (Strat == "twise") {
            unsigned T = 0;
            try {
                T = static_cast<unsigned>(std::stoul(Args[3]));
            } catch (const std::invalid_argument &E) {
                std::cerr << "Invalid value for t: " << Args[3] << '\n';
                return 1;
            }   

            if (T < 1 || T > 3) {
                std::cerr << "Value of t must be between 1 and 3 (inclusive). Provided: " << T << '\n';
                return 1;
            }

            auto Interactions = bdd::sample::EnumerateInteractions(Manager, FinalBDD, T, ValidCondfigs);
            auto S   = bdd::sample::TSample(ValidCondfigs, Interactions);

            // Validate the generated configurations against the BDD
            for (const auto &Config : S) {
                std::vector<std::pair<oxidd::var_no_t, bool>> ValidTSamples;
                ValidTSamples.reserve(Config.size());
                for(oxidd::var_no_t V = 0; V < Config.size(); ++V) {
                    ValidTSamples.emplace_back(V, Config[V]);
                }
                if(!FinalBDD.eval(ValidTSamples)){
                    std::cerr << "Generated an invalid configuration at iteration. This should not happen.\n";
                    return 1;
                }
            }

            // Validate the configurations against the coverage of each interaction 
            for (const auto &I: Interactions) {
                bool CoveredInts = false;
                for (const auto &Config : S) {
                    if(bdd::sample::CoverageCheck(Config, I)) {
                        CoveredInts = true; break;
                    }
                }
                if(!CoveredInts) {
                    std::cerr << "Sample did not cover all interactions" << "\n";
                }
            }

            std::string TPath = (T == 1 ? "T1" : (T == 2 ? "T2" : "T3")); //NOLINT
            std::string TFilePath = "bindings/python/Interplay_ML/Samples/" + TPath + "/" + Sys + "_TWiseSample.csv";
            std::filesystem::create_directories(std::filesystem::path(TFilePath).parent_path());
            bdd::sample::WriteSampleCSV(Manager, S, TFilePath);

        } else if (Strat == "random") {
            unsigned SampleSize = 0;
            try {
                SampleSize = static_cast<unsigned>(std::stoul(Args[3]));
            } catch (const std::invalid_argument &E) {
                std::cerr << "Invalid value for sample size: " << Args[3] << '\n';
                return 1;
            }

            unsigned Seed = 0;
            try {
                Seed = static_cast<unsigned>(std::stoul(Args[4]));
            } catch (const std::invalid_argument &E) {
                std::cerr << "Invalid value for seed: " << Args[4] << '\n';
                return 1;
            }

            bdd::sample::SeedRng(Seed);
            std::vector<std::vector<bool>> Samples;
            Samples.reserve(SampleSize);
            for (size_t I = 0; I < SampleSize; ++I) {
                auto S = generateConfiguration(
                    Manager, 
                    FinalBDD, 
                    Factory,
                    &Factory.SatMap
                );

                Samples.push_back(S);
            }
        std::string FilePath = "bindings/python/Interplay_ML/Samples/RSSeed/" + Sys + "_" + std::to_string(Seed) + ".csv";
        std::filesystem::create_directories(std::filesystem::path(FilePath).parent_path());
        if (Seed == 1) { bdd::sample::WriteFrequencyCSV(FinalBDD, Samples); }
        bdd::sample::WriteSampleCSV(Manager, Samples, FilePath);

        } else {
            std::cerr << "Invalid sampling strategy: " << Strat << ". Use 'twise' or 'random'.\n";
            return 1;
        }
    }

    if (Command == "sat_count") {
        double Count = FinalBDD.sat_count_double(Manager.num_vars());
        std::cout << Count << "\n";
        return 0;
    }

    return 0;

    //------------Plotting--------------//

    std::string ConfigCSV = "Random_Sampler/scripts/Configs.csv";
    std::string ConfigCmd = "/Users/oracionoftime/.pyenv/versions/vara-feature-env/bin/python3 Random_Sampler/scripts/plot_dist.py " + ConfigCSV;
    int Configs = std::system(ConfigCmd.c_str());

    if(Configs != 0) {
        std::cerr << "Error executing command" << '\n';
    } else {
        std::cerr << "Histogram generated successfully." << '\n';
    }

    return 0;
}
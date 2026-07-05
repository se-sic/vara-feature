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
        * argv: -- <path_to_feature_model.xml>
        2.1 From the repo root: 
            * ./build/bin/random_sampler argv[1]
            * Example: ./build/bin/random_sampler Random_Sampler/examples/Polly.xml
*/


int main(int argc, char* argv[]) noexcept(false){ 

    //--------------Parsing--------------//

    if (argc < 2) {
        std::cerr << "Usage: ./my_program <feature_model.xml>\n";
        return 1;
    }

    std::vector<std::string> Args(argv + 1, argv + argc);

    //Parse XML to a BDD-readable format
    std::unique_ptr<vara::feature::FeatureModel> Fd = bdd::sample::ParseXML(Args[0]);
    std::cout << "Feature Model loaded successfully from: " << Args[0] <<'\n';

    // Create BDD factory and convert feature model to BDD
    bdd::sample::BDDFactory Factory;
    oxidd::bdd_function FinalBDD = Factory.modelToBdd(*Fd);
    oxidd::bdd_manager Manager = FinalBDD.containing_manager(); 
    std::cout << "BDD constructed successfully." << '\n';

    //Enumeration of all configurations
    auto ValidCondfigs = bdd::sample::EnumerateConfigs(Manager, FinalBDD);
    double Expected = FinalBDD.sat_count_double(Manager.num_vars());
    std::cout << "Valid Configs: " << ValidCondfigs.size() << " (expected " << Expected << ")\n";
    if (ValidCondfigs.size() != static_cast<std::size_t>(Expected)) {
        std::cerr << "Enumeration count mismatch\n";
    }

    //Enumeration of all interactions
    auto Interactions = bdd::sample::EnumerateInteractions(Manager, FinalBDD, 2);
    std::size_t Num = Manager.num_vars();
    std::size_t UpperBound = (Num * (Num -1) / 2)* 4;
    std::cout << "Feasibel 2-wise interaction " << Interactions.size() << " (upper bound " << UpperBound << ")\n";

    //--------------T-Wise-Sampling--------------//

    auto S = bdd::sample::TSample(ValidCondfigs, Interactions);
    const auto SampleSize = S.size();

    // Validate the generated configurations against the BDD
    for (const auto &Config : S) {
        std::vector<std::pair<oxidd::var_no_t, bool>> ValidTSamples;
        ValidTSamples.reserve(Config.size());
        for(oxidd::var_no_t V = 0; V < Config.size(); ++V) {
            ValidTSamples.emplace_back(V, Config[V]);
        }
        if(!FinalBDD.eval(ValidTSamples)){
            std::cerr << "Generated an invalid configuration at iteration. This should not happen.\n";
            return 0;
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
            std::cout << "Sample did not cover all interactions" << "\n";
        }
    }

    std::string TFilePath = "Random_Sampler/scripts/TWiseSample.csv";
    bdd::sample::WriteSampleCSV(Manager, S, TFilePath);

    //--------------Uniform Random Sampling--------------//

    //Number of samples to generate
    size_t N = SampleSize; 
    // Generate multiple samples and update frequency counts
    std::vector<std::vector<bool>> Samples;
    for(size_t I=0; I<N; ++I) {
        auto S = generateConfiguration(
            Manager, 
            FinalBDD, 
            Factory,
            &Factory.SatMap
        );

        Samples.push_back(S);
    }
    
    bdd::sample::WriteFrequencyCSV(FinalBDD, Samples);
    std::string FilePath = "Random_Sampler/scripts/RandSample.csv";
    bdd::sample::WriteSampleCSV(Manager, Samples, FilePath);

    //------------Plotting--------------//

    std::string ConfigCSV = "Random_Sampler/scripts/Configs.csv";
    std::string ConfigCmd = "/Users/oracionoftime/.pyenv/versions/vara-feature-env/bin/python3 Random_Sampler/scripts/plot_dist.py " + ConfigCSV;
    int Configs = std::system(ConfigCmd.c_str());

    if(Configs != 0) {
        std::cerr << "Error executing command" << '\n';
    } else {
        std::cout << "Histogram generated successfully." << '\n';
    }

    return 1;
}
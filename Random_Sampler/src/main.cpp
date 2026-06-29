#include "BDDSampler.h"
#include "../../BDD/include/BDDFactory.h"
#include "EnumerateConfigs.h"
#include "EnumerateInteractions.h"
#include "InteractionCoverage.h"
#include "Plotter.h"
#include "TWiseSampler.h"
#include "oxidd/util.hpp"
#include "vara/Feature/FeatureModelParser.h"
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
extern "C" {
#include <oxidd/bdd.hpp>
}

using std::string;
using std::vector;


int main(int argc, char* argv[]) noexcept(false){ 

    if (argc < 2) {
        std::cerr << "Usage: ./my_program <feature_model.xml>\n";
        return 1;
    }

    std::vector<std::string> Args(argv + 1, argv + argc);
    std::string FilePath = Args[0];

    // Lambda function to load and parse the feature model
    std::unique_ptr<vara::feature::FeatureModel> Fd = [&]() {
        // Read the file content
        std::ifstream FileIn(FilePath);
        if (!FileIn) {
            throw std::runtime_error("Could not open file: " + FilePath);
        }
        std::ostringstream Oss;
        Oss << FileIn.rdbuf();
        std::string XMLContent = Oss.str();

        // Parse the content
        vara::feature::FeatureModelXmlParser Parser(XMLContent);

        // Verify if the feature model is valid
        auto Verify = Parser.verifyFeatureModel();
        if(!Verify) {
            throw std::runtime_error("Error parsing XML: verification failed");
        }   

        // Build the feature model object
        auto Fm = Parser.buildFeatureModel();
        if(!Fm) {
            throw std::runtime_error("Error building Feature Model: ");
        }

        return Fm;
    }();

    std::cout << "Feature Model loaded successfully from: " << FilePath <<'\n';

    // Create BDD factory and convert feature model to BDD
    bdd::sample::BDDFactory Factory;
    oxidd::bdd_function FinalBDD = Factory.modelToBdd(*Fd);
    oxidd::bdd_manager Manager = FinalBDD.containing_manager();
    
    std::cout << "BDD constructed successfully." << '\n';

    //Enumeration of all configurations
    auto Pool = bdd::sample::EnumerateConfigs(Manager, FinalBDD);
    double Expected = FinalBDD.sat_count_double(Manager.num_vars());
    std::cout << "Valid Configs: " << Pool.size() << " (expected " << Expected << ")\n";
    if (Pool.size() != static_cast<std::size_t>(Expected)) {
        std::cerr << "Enumeration count mismatch\n";
    }

    //Enumeration of all interactions
    auto Interactions = bdd::sample::EnumerateInteractions(Manager, FinalBDD, 2);
    std::size_t Num = Manager.num_vars();
    std::size_t UpperBound = (Num * (Num -1) / 2)* 4;
    std::cout << "Feasibel 2-wise interaction " << Interactions.size() << " (upper bound " << UpperBound << ")\n";


    // Uniform Random Sampling //
    std::vector<size_t> BellCounts;
    std::map<std::vector<bool>, int> CountConfig;

    size_t N = 100; // Number of samples to generate
       
    BellCounts.reserve(N);

    // Generate multiple samples and update frequency counts
    for(size_t I=0; I<N; ++I) {
        auto S = generateConfiguration(
            Manager, 
            FinalBDD, 
            Factory,
            &Factory.SatMap
        );

        // Validate the generated configuration against the BDD
        std::vector<std::pair<oxidd::var_no_t, bool>> ValidSamples; // NOLINT
        ValidSamples.reserve(S.size());
        for(oxidd::var_no_t V = 0; V < S.size(); ++V) {
            ValidSamples.emplace_back(V, S[V]);
        }
        if(!FinalBDD.eval(ValidSamples)){
            std::cerr << "Generated an invalid configuration at iteration " << I << "This should not happen.\n";
            std::abort();
        }

        size_t Count = 0;
        for(auto [v, val] : llvm::enumerate(S)) {
            if(val) { ++Count; };
        }

        BellCounts.push_back(Count);

        int &FeatureCount = CountConfig[std::move(S)];
        ++FeatureCount;
    }


    {
        std::ofstream Out("Random_Sampler/scripts/bell.csv");
        Out << "ActiveFeatures\n";
        for(size_t C : BellCounts) {
            Out << C << '\n';
        }
    }

    std::ofstream Out("Random_Sampler/scripts/Configs.csv");
    Out << "ConfigID,Count\n";
    {
        int Id = 0;
        for (auto &[_Feat, Count] : CountConfig) {
            Out << Id++ << "," << Count << '\n';
        }
    }
    Out.close();


    // TWise Sampling //
    for(size_t I=0; I<1; ++I) {
        auto S = bdd::sample::TSample(Pool, Interactions);
        // Validate the generated configurations against the BDD
        for (const auto &Config : S) {
            std::vector<std::pair<oxidd::var_no_t, bool>> ValidTSamples;
            ValidTSamples.reserve(Config.size());
            for(oxidd::var_no_t V = 0; V < Config.size(); ++V) {
                ValidTSamples.emplace_back(V, Config[V]);
            }
            if(!FinalBDD.eval(ValidTSamples)){
                std::cerr << "Generated an invalid configuration at iteration " << I << "This should not happen.\n";
                std::abort();
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
    }


    //std::string BellCSV = "Random_Sampler/scripts/bell.csv";
    std::string ConfigCSV = "Random_Sampler/scripts/Configs.csv";
    //std::string BellCmd = "/Users/oracionoftime/.pyenv/versions/vara-feature-env/bin/python3 Random_Sampler/scripts/plot_dist.py " + BellCSV;
    std::string ConfigCmd = "python3 Random_Sampler/scripts/plot_dist.py " + ConfigCSV;

    //int Bell = std::system(BellCmd.c_str());
    int Configs = std::system(ConfigCmd.c_str());

    if(Configs != 0) {
        std::cerr << "Error executing command" << '\n';
    } else {
        std::cout << "Histogram generated successfully." << '\n';
    }

    return 1;
}
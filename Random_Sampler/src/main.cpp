#include "BDDSampler.h"
#include "../../BDD/include/BDDFactory.h"
#include "CoverageIntegration.h" 
#include "Plotter.h"
#include "vara/Feature/FeatureModelParser.h"
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
extern "C" {
#include <oxidd/bdd.hpp>
}

using std::string;
using std::vector;

namespace {
static bool isFalse(const oxidd::bdd_function &F) {
    if (F.is_invalid()) {return false; } 
    return F == (F & ~F);
}

void analyseFeatures(const oxidd::bdd_function &BDD, oxidd::bdd_manager &Manager, const vara::feature::FeatureModel &Model){
    for (const auto *Feat : Model.features()) {
        const llvm::StringRef &Name = Feat->getName();

        auto AbstractVariable = Manager.name_to_var(Name);

        if (!AbstractVariable) {
            continue;
        }

        auto Variable = Manager.var(*AbstractVariable);

        auto WithFeature = BDD & Variable;

        auto WithoutFeature = BDD & ~Variable;

         if (isFalse(WithFeature)) {
            std::cout << "DEAD  : " << Feat->getName().str() << "\n";
        } else if (isFalse(WithoutFeature)) {
            std::cout << "CORE  : " << Feat->getName().str() << "\n";
        }
        
    }
}
} // namespace

int main(int argc, char* argv[]) noexcept(false){ 

    if (argc < 3) {
        std::cerr << "\033[31m" << "Usage: ./my_program <feature_model.xml>\033[0m\n";
        return 1;
    }

    std::vector<std::string> Args(argv + 1, argv + argc);
    std::string FilePath = Args[0];

    size_t SampleSize = 0;

        try {
            SampleSize = std::stoi(Args[1]);
        } catch (const std::invalid_argument& E) {
            std::cerr << "\033[31m" << "Error: Second argument must be an integer.\033[0m\n";
            return 1;
        } catch (const std::out_of_range& E) {
            std::cerr << "\033[31m" << "Error: Integer value is out of range.\033[0m\n";
            return 1;
        }
        

    // Lambda function to load and parse the feature model
    std::unique_ptr<vara::feature::FeatureModel> Fd = [&]() {
        // Read the file content
        std::ifstream FileIn(FilePath);
        if (!FileIn) {
            throw std::runtime_error("\033[31m Could not open file: \033[0m" + FilePath);
        }
        std::ostringstream Oss;
        Oss << FileIn.rdbuf();
        std::string XMLContent = Oss.str();

        // Parse the content
        vara::feature::FeatureModelXmlParser Parser(XMLContent);

        // Verify the feature model is valid
        auto Verify = Parser.verifyFeatureModel();
        if(!Verify) {
            throw std::runtime_error("\033[31m Error parsing XML: verification failed \033[0m");
        }   

        // Build the feature model object
        auto Fm = Parser.buildFeatureModel();
        if(!Fm) {
            throw std::runtime_error("\033[31mError building Feature Model: \033[0m");
        }

        return Fm;
    }();

    std::cout << "Feature Model loaded successfully from: " << FilePath <<"\033[0m\n";

    // Create BDD factory and convert feature model to BDD
    bdd::sample::BDDFactory Factory;
    oxidd::bdd_function FinalBDD = Factory.modelToBdd(*Fd);
    oxidd::bdd_manager Manager = FinalBDD.containing_manager();
    analyseFeatures(FinalBDD, Manager, *Fd);
    
    std::cout << "BDD constructed successfully." << '\n';

    // Generate a single sample configuration
    std::vector<bool> Sample = generateConfiguration(
        Manager, 
        FinalBDD, 
        Factory,
        &Factory.SatMap
    );

    std::vector<size_t> BellCounts;
    std::map<std::vector<bool>, int> CountConfig;

    size_t N = SampleSize; // Number of samples to generate

    BellCounts.reserve(N);

    std::vector<std::vector<bool>> AllSamples;  
    AllSamples.reserve(N);                       

    // Generate multiple samples and update frequency counts
    for(size_t I=0; I<N; ++I) {
        auto S = generateConfiguration(
            Manager, 
            FinalBDD, 
            Factory,
            &Factory.SatMap
        );

        AllSamples.push_back(S); 

        size_t Count = 0;
        for (auto it : llvm::enumerate(S)) {
            auto v = it.index();
            const auto &val = it.value();
            if(val) { ++Count; };
        }

        BellCounts.push_back(Count);

        int &FeatureCount = CountConfig[std::move(S)];
        ++FeatureCount;

        if ((I + 1) % 10000 == 0) {
            std::cout << "  Generated " << (I + 1) << " samples\r" << std::flush;
        }
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

    coverage::CoverageEvaluator Evaluator(*Fd, FinalBDD, Manager, Factory);

    std::vector<size_t> Checkpoints = {
    N/10,   // 200
    N/4,    // 500
    N/2,    // 1000
    3*N/4,  // 1500
    N       // 2000
    };

    for (size_t T : {1, 2, 3}) { 
        std::cout << "\033[35m" << "\nEvaluating " << T << "-wise coverage\033[0m";
        
        // Incremental evaluation
        auto Results = Evaluator.evaluateIncremental(AllSamples, T, Checkpoints);
        
        // Export results
        std::string Filename = "Random_Sampler/scripts/coverage_" + std::to_string(T) + "wise.csv";
        Evaluator.exportToCSV(Filename, Results, Checkpoints);
        
        std::cout << "Results saved to " << Filename << "\n";
        
        // Print final coverage (100% sample)
        std::cout << "\033[35m" <<"\nFinal " << T << "-wise coverage (N=" << N << "):\n";
        for (const auto& [metricName, coverages] : Results) {
            if (!coverages.empty()) {
                double FinalCoverage = coverages.back();
                std::cout << "\033[35m" << "  " << std::setw(25) << std::left << metricName 
                          << ": " << std::fixed << std::setprecision(2)
                          << (FinalCoverage * 100.0) << "%\033[0m\n";
            }
        }
    }

    std::string ConfigCSV = "Random_Sampler/scripts/Configs.csv";

    
    std::string PlotPath = "Random_Sampler/scripts/plot_dist.py";
    std::string ConfigCmd = "python3 " + PlotPath + " " + ConfigCSV;

    std::cout << "Executing: " << ConfigCmd << '\n';
    int Configs = std::system(ConfigCmd.c_str());

    if(Configs != 0) {
        std::cerr << "\033[31mError executing command" << "\033[0m\n";
    } else {
        std::cout << "Histogram generated successfully." << '\n';
    }

    std::string CoveragePath = "Random_Sampler/scripts/plot_coverage.py";
    std::string CoveragePlotCmd = "python3 " + CoveragePath + " " + ConfigCSV;
    int CoveragePlot = std::system(CoveragePlotCmd.c_str());
    
    if (CoveragePlot != 0) {
        std::cerr << "Note: Coverage plot script not found or failed.\n";
    }

    std::cout << "Generated " << N << " random samples\n";
    std::cout << "Unique configurations: " << CountConfig.size() << "\n";
    std::cout << "Coverage results saved to:\n";
    std::cout << "  - Random_Sampler/scripts/coverage_1wise.csv\n";
    std::cout << "  - Random_Sampler/scripts/coverage_2wise.csv\n";

    return 0;
}




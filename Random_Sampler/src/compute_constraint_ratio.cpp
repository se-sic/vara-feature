#include "BDDFactory.h"
#include "../tests/utilities/test_helpers.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelParser.h"
#include <algorithm>
#include <cmath>
#include <fstream>   
#include <iomanip>
#include <iostream>
#include <regex>
#include <vector>

struct ModelStats {
    std::string Name;
    size_t NumFeatures{};      // F_i
    double NumValidConfigs{};  // V_i (using double for large numbers)
    double UnconstrainedSpace{}; // 2^(F_i - 1)
    double Ratio{};            // R_i
    
    [[nodiscard]] std::string constraintLevel() const {
        if (Ratio >= 0.5) { 
            return "Weak";
        }

        if (Ratio >= 0.1) { 
            return "Moderate";
        }

        if (Ratio >= 0.01) { 
            return "Strong";
        }

        return "Very Strong";
    }
};

size_t countFeaturesInXML(const std::string& XmlPath) {
    std::ifstream File(XmlPath);
    if (!File.is_open()) {
        std::cerr << "Warning: Cannot open " << XmlPath << " to count features\n";
        return 0;
    }
    
    std::string Content((std::istreambuf_iterator<char>(File)),
                        std::istreambuf_iterator<char>());
    
    // Count <configurationOption> tags (each = one feature)
    std::regex FeatureRegex("<configurationOption>");
    auto Begin = std::sregex_iterator(Content.begin(), Content.end(), FeatureRegex);
    auto End = std::sregex_iterator();
    
    return std::distance(Begin, End);
}

class ConstraintAnalyzer {
public:
    ConstraintAnalyzer() = default;
    
    static ModelStats analyzeModel(const std::string& ModelPath, const std::string& ModelName) {
        ModelStats Stats;
        Stats.Name = ModelName;
        
        auto FeatureModel = test_utils::loadFeatureModel(ModelPath);
        
        // Build BDD
        bdd::sample::BDDFactory Factory;
        auto Bdd = Factory.modelToBdd(*FeatureModel);
        auto Manager = Bdd.containing_manager(); 
        
        // Get counts
        size_t XmlFeatures = countFeaturesInXML(ModelPath);  
        size_t BddVars = Manager.num_vars();  
        
        Stats.NumFeatures = XmlFeatures;
        
        Stats.NumValidConfigs = Bdd.sat_count_double(BddVars);
        
        Stats.UnconstrainedSpace = std::pow(2.0, XmlFeatures - 1);
        
        Stats.Ratio = Stats.NumValidConfigs / Stats.UnconstrainedSpace;
        
        std::cout << ModelName << " \n";
        std::cout << "Features (original): " << XmlFeatures << "\n";
        std::cout << "BDD variables: " << BddVars << "\n";
        std::cout << "Valid configs: " << Stats.NumValidConfigs << "\n";
        
        return Stats;
    }

    static ModelStats debuganalyzeModel(const std::string& ModelPath, const std::string& ModelName) {
    ModelStats Stats;
    Stats.Name = ModelName;
    
    auto FeatureModel = test_utils::loadFeatureModel(ModelPath);
    
    // Get total features
    size_t TotalFeatures = FeatureModel->size();
    std::cout << "\n=== " << ModelName << " ===\n";
    std::cout << "Total features: " << TotalFeatures << "\n";

    const std::vector<std::string> FakeRoot = {
                "BerkeleyDBC", "Hipacc", "LLVM", "lrzip", "x264"
            };
    
    // Build BDD
    bdd::sample::BDDFactory Factory;
    auto Bdd = Factory.modelToBdd(*FeatureModel);
    oxidd::bdd_manager Manager = Bdd.containing_manager();

    // Count mandatory vs optional
    size_t MandatoryCount = computeMandatoryFeatures(*FeatureModel, Manager, Bdd).size();

    if (std::ranges::contains(FakeRoot, ModelName)) {
        TotalFeatures--;
        MandatoryCount--;
    }

    size_t OptionalCount = TotalFeatures - MandatoryCount;
    
    std::cout << "Mandatory: " << MandatoryCount << "\n";
    std::cout << "Optional: " << OptionalCount << "\n";
    
    // Try different counting methods
    double Count1 = Bdd.sat_count_double(TotalFeatures);
    double Count2 = Bdd.sat_count_double(OptionalCount);
    double Count3 = Bdd.sat_count_double(TotalFeatures - 1);
    
    std::cout << "Count with all features (" << TotalFeatures << "): " << Count1 << "\n";
    std::cout << "Count with optional (" << OptionalCount << "): " << Count2 << "\n";
    std::cout << "Count with (total-1) (" << (TotalFeatures-1) << "): " << Count3 << "\n";
    
    Stats.NumFeatures = TotalFeatures;
    Stats.NumValidConfigs = Count1;
    
    return Stats;
}

static std::set<size_t> computeMandatoryFeatures(vara::feature::FeatureModel& FeatureModel, oxidd::bdd_manager& Manager, oxidd::bdd_function& BDD) {
    std::set<size_t> MandatoryFeatures;
    
    std::cout << "Detecting core features (exact)...\n";
    
    // For each feature, check if it's always selected
    // A feature is core if (BDD & NOT feature) is unsatisfiable
    for (size_t I = 0; I < FeatureModel.size(); ++I) {
        try {
            // Get NOT feature_i (negated variable)
            auto NotVarI = Manager.not_var(static_cast<oxidd::var_no_t>(I));
            
            // Compute: BDD & NOT feature_i
            auto Constrained = BDD & NotVarI;
            
            // If unsatisfiable, feature must always be true (core)
            if (!Constrained.satisfiable()) {
                MandatoryFeatures.insert(I);
            }
            
        } catch (const std::exception& E) {
            std::cerr << "\033[31m" <<"Warning: Error checking feature " << I 
                      << " for core: " << E.what() << "\033[0m\n\n";
            continue;
        }
    }

    std::cout << "Mandatory features: ";
    for (size_t Index : MandatoryFeatures) {
        std::cout << Manager.var_name(Index) << " ";
    }
    std::cout << '\n';

    
    
    std::cout << "Found " << MandatoryFeatures.size() << " core features\n";
    return MandatoryFeatures;
}
    
    static void printStats(const std::vector<ModelStats>& AllStats) {        
        std::cout << std::left 
          << std::setw(20) << "System"
          << std::setw(12) << "Features"
          << std::setw(18) << "Valid Configs"
          << std::setw(18) << "Unconstrained"
          << std::setw(17) << "R_i"
          << std::setw(15) << "Constraint"
          << "\n";
        std::cout << std::string(95, '-') << "\n";
        
        for (const auto& Stats : AllStats) {
            std::cout << std::left 
              << std::setw(20) << Stats.Name
              << std::setw(12) << Stats.NumFeatures
              << std::setw(18) << std::scientific << std::setprecision(2) << Stats.NumValidConfigs
              << std::setw(18) << std::scientific << std::setprecision(2) << Stats.UnconstrainedSpace
              << std::setw(17) << std::scientific << std::setprecision(10) << Stats.Ratio
              << std::setw(15) << Stats.constraintLevel()
              << "\n";
        }
        
        std::cout << "\n";
        printSummary(AllStats);
    }
    
    static void printSummary(const std::vector<ModelStats>& AllStats) {
        
        // Compute statistics
        std::vector<double> Ratios;
        Ratios.reserve(AllStats.size());
        for (const auto& S : AllStats) {
            Ratios.push_back(S.Ratio);
        }
        
        std::ranges::sort(Ratios);
        
        double Min = Ratios.front();
        double Max = Ratios.back();
        double Median = Ratios[Ratios.size() / 2];
        double Mean = std::accumulate(Ratios.begin(), Ratios.end(), 0.0) / static_cast<double>(Ratios.size());
        
        // Count by category
        int Weak = 0;
        int Moderate = 0;
        int Strong = 0;
        int VeryStrong = 0;
        for (const auto& S : AllStats) {
            if (S.Ratio >= 0.5) { Weak++;
            } else if (S.Ratio >= 0.1) { Moderate++;
            } else if (S.Ratio >= 0.01) { Strong++;
            } else { VeryStrong++;
            }
        }
        
        std::cout << "R_i Statistics:\n";
        std::cout << "  Min:    " << std::fixed << std::setprecision(6) << Min << "\n";
        std::cout << "  Max:    " << std::fixed << std::setprecision(6) << Max << "\n";
        std::cout << "  Mean:   " << std::fixed << std::setprecision(6) << Mean << "\n";
        std::cout << "  Median: " << std::fixed << std::setprecision(6) << Median << "\n\n";
        
        std::cout << "Constraint Strength Distribution:\n";
        std::cout << "  Weak (R_i ≥ 0.5):        " << Weak << " systems\n";
        std::cout << "  Moderate (0.1 ≤ R_i < 0.5): " << Moderate << " systems\n";
        std::cout << "  Strong (0.01 ≤ R_i < 0.1):  " << Strong << " systems\n";
        std::cout << "  Very Strong (R_i < 0.01):   " << VeryStrong << " systems\n\n";
        
        std::cout << "Interpretation:\n";
        std::cout << "  - R_i close to 1.0: Few constraints, most combinations valid\n";
        std::cout << "  - R_i close to 0.0: Heavy constraints, few combinations valid\n";
        std::cout << "  - Range: " << std::fixed << std::setprecision(1) 
                  << (Max / Min) << "x difference between most and least constrained\n\n";
    }
    
    static void exportCSV(const std::vector<ModelStats>& AllStats, const std::string& Filename) {
        std::ofstream Out(Filename);
        
        Out << "System,Features,ValidConfigs,UnconstrainedSpace,Ratio,ConstraintLevel\n";
        
        for (const auto& Stats : AllStats) {
            Out << Stats.Name << ","
                << Stats.NumFeatures << ","
                << std::scientific << std::setprecision(10) << Stats.NumValidConfigs << ","
                << std::scientific << std::setprecision(10) << Stats.UnconstrainedSpace << ","
                << std::fixed << std::setprecision(10) << Stats.Ratio << ","
                << Stats.constraintLevel() << "\n";
        }
        
        std::cout << "Results exported to: " << Filename << "\n";
    }
};
int main(int argc, char** argv) {
    // Define your models

    std::vector<std::pair<std::string, std::string>> Models = {
        // {"Random_Sampler/examples/FeatureModel/7z.xml", "7z"},
        // {"Random_Sampler/examples/FeatureModel/BerkeleyDBC.xml", "BerkeleyDBC"},
        // {"Random_Sampler/examples/FeatureModel/Dune.xml", "Dune"},
        // {"Random_Sampler/examples/FeatureModel/Hipacc.xml", "Hipacc"},
        // {"Random_Sampler/examples/FeatureModel/JavaGC.xml", "JavaGC"},
        // {"Random_Sampler/examples/FeatureModel/LLVM.xml", "LLVM"},
        // {"Random_Sampler/examples/FeatureModel/lrzip.xml", "lrzip"},
        // {"Random_Sampler/examples/FeatureModel/Polly.xml", "Polly"},
        // {"Random_Sampler/examples/FeatureModel/VP9.xml", "VP9"},
        // {"Random_Sampler/examples/FeatureModel/x264.xml", "x264"},


        {"Random_Sampler/examples/FeatureModel/AJStats.xml", "AJStats"},
        {"Random_Sampler/examples/FeatureModel/Curl.xml", "Curl"},
        {"Random_Sampler/examples/FeatureModel/HSMGP.xml", "HSMGP"},
        {"Random_Sampler/examples/FeatureModel/HSQLDB.xml", "HSQLDB"},
        {"Random_Sampler/examples/FeatureModel/HyTeG.xml", "HyTeG"},
        {"Random_Sampler/examples/FeatureModel/PKJab.xml", "PKJab"},
        {"Random_Sampler/examples/FeatureModel/SQLite.xml", "SQLite"},
        {"Random_Sampler/examples/FeatureModel/TriMesh.xml", "TriMesh"},
        {"Random_Sampler/examples/FeatureModel/WGet.xml", "WGet"},
        {"Random_Sampler/examples/FeatureModel/clasp.xml", "clasp"},
        {"Random_Sampler/examples/FeatureModel/z3.xml", "z3"},
    };
    
    std::vector<ModelStats> AllStats;
    
    std::cout << "Analyzing " << Models.size() << " feature models...\n\n";
    
    for (const auto& [path, name] : Models) {
        auto Stats = ConstraintAnalyzer::analyzeModel(path, name);
        AllStats.push_back(Stats);
    }
    
    // Print results
    ConstraintAnalyzer::printStats(AllStats);
    
    // Export to CSV
    ConstraintAnalyzer::exportCSV(AllStats, "constraint_ratios.csv");
    
    return 0;
}

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
    double LogRatio{};         //log10(R_i)
    
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

std::unique_ptr<vara::feature::FeatureModel> loadFeatureModel(const std::string& FilePath) {
    std::ifstream FileIn(FilePath);
    if (!FileIn) {
        throw std::runtime_error("Could not open file: " + FilePath);
    }

    std::ostringstream Oss;
    Oss << FileIn.rdbuf();
    std::string XMLContent = Oss.str();

    vara::feature::FeatureModelXmlParser Parser(XMLContent);

    auto Verify = Parser.verifyFeatureModel();
    if (!Verify) {
        throw std::runtime_error("Error parsing XML: verification failed");
    }

    auto Fm = Parser.buildFeatureModel();
    if (!Fm) {
        throw std::runtime_error("Error building feature model");
    }

    return Fm;
}

size_t countFeaturesInXML(const std::string& XmlPath) {
    std::ifstream File(XmlPath);
    if (!File.is_open()) {
        std::cerr << "Warning: Cannot open " << XmlPath << " to count features\n";
        return 0;
    }
    
    std::string Content((std::istreambuf_iterator<char>(File)),
                        std::istreambuf_iterator<char>());
    
    // Count <configurationOption> tags 
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

        Stats.LogRatio = std::log10(Stats.NumValidConfigs)
               - (static_cast<double>(XmlFeatures) - 1.0) * std::log10(2.0);
        
        std::cout << ModelName << " \n";
        std::cout << "Features (original): " << XmlFeatures << "\n";
        std::cout << "BDD variables: " << BddVars << "\n";
        std::cout << "Valid configs: " << Stats.NumValidConfigs << "\n";
        
        return Stats;
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
    
    static void appendResultsToCsv(const ModelStats& Stats, const std::string& OutputCsv) {
        bool WriteHeader = false;
        {
            std::ifstream In(OutputCsv);
            WriteHeader = !In.good() || In.peek() == std::ifstream::traits_type::eof();
        }

        std::ofstream Out(OutputCsv, std::ios::app);

        if (!Out) {
            throw std::runtime_error("Could not open output CSV: " + OutputCsv);
        }

        if (WriteHeader) {
            Out << "system,valid_configs,num_features,ratio,log10_ratio,constraint_level\n";
        }
            
        Out << Stats.Name << ","
            << std::scientific << std::setprecision(6) << Stats.NumValidConfigs << ","
            << Stats.NumFeatures << ","
            << std::scientific << std::setprecision(6) << Stats.Ratio << ","
            << std::fixed << std::setprecision(4) << Stats.LogRatio << ","
            << Stats.constraintLevel() << "\n";
    }
};

int main(int argc, char** argv) {
    try {
            if (argc < 4) {
                std::cerr << "Usage: ./experiment_runner <feature_model.xml> <name> "
                            "<output.csv>\n";
                return 1;
            }

            std::string FilePath = argv[1];
            std::string System = argv[2];
            std::string OutputCsv = argv[3];

            auto FeatureModel = loadFeatureModel(FilePath);

            bdd::sample::BDDFactory Factory;
            oxidd::bdd_function FinalBDD = Factory.modelToBdd(*FeatureModel);
            oxidd::bdd_manager Manager = FinalBDD.containing_manager();
        
            
            // Print results
            //ConstraintAnalyzer::printStats(AllStats);

            ModelStats Stats = ConstraintAnalyzer::analyzeModel(FilePath, System);
            
            // Export to CSV
            ConstraintAnalyzer::appendResultsToCsv(Stats, OutputCsv);
            
            return 0;
        } catch (const std::exception& E) {
            std::cerr << "ERROR: " << E.what() << "\n";
            return 1;
        }
}

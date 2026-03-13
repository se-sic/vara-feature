#include "BDDFactory.h"
#include "BDDSampler.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelParser.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>

class ConfigComparator {
public:
    struct Config {
        std::map<std::string, bool> Features;
        
        bool operator<(const Config& Other) const {
            return Features < Other.Features;
        }
        
        bool operator==(const Config& Other) const {
            return Features == Other.Features;
        }
        
        [[nodiscard]] std::string toString() const {
            std::string Result = "{";
            bool First = true;
            for (const auto& [name, value] : Features) {
                if (!First) { 
                    Result += ", ";
                }
                Result += name + "=" + (value ? "T" : "F");
                First = false;
            }
            Result += "}";
            return Result;
        }
        
        [[nodiscard]] std::string toSelectedOnly() const {
            std::string Result;
            for (const auto& [name, value] : Features) {
                if (value) {
                    if (!Result.empty()) { 
                        Result += ",";
                    }
                    Result += name;
                }
            }
            return Result.empty() ? "(none)" : Result;
        }
    };
    
    // Parse the measurement XML to extract configs
    std::set<Config> loadPaperConfigs(
    const std::string& XmlPath,
    const std::vector<std::string>& AllFeatureNames) {
    
    std::set<Config> Configs;
    std::ifstream File(XmlPath);
    if (!File.is_open()) {
        std::cerr << "Error: Cannot open " << XmlPath << "\n";
        return Configs;
    }
    
    // Read whole file into a string
    std::string Content((std::istreambuf_iterator<char>(File)),
                         std::istreambuf_iterator<char>());
    
    // Regex now uses [\s\S]*? to match across newlines
    std::regex ConfigRegex(R"(<data columname="Configuration">([\s\S]*?)</data>)");
    
    std::cout << "Parsing XML...\n";
    
    auto Begin = std::sregex_iterator(Content.begin(), Content.end(), ConfigRegex);
    auto End = std::sregex_iterator();
    
    // Debug: number of matches found
    size_t MatchCount = 0;
    for (auto It = Begin; It != End; ++It) {
        MatchCount++;
        std::string ConfigStr = (*It)[1].str();
        
        // Trim whitespace (including newlines)
        ConfigStr.erase(0, ConfigStr.find_first_not_of(" \t\r\n"));
        ConfigStr.erase(ConfigStr.find_last_not_of(" \t\r\n") + 1);
        
        // Remove trailing comma if present
        if (!ConfigStr.empty() && ConfigStr.back() == ',') {
            ConfigStr.pop_back();
        }
        
        Config Config = parseConfigString(ConfigStr, AllFeatureNames);
        Configs.insert(Config);
    }
    
    std::cout << "Found " << MatchCount << " configuration elements.\n";
    std::cout << "Loaded " << Configs.size() << " configs from paper\n";
    return Configs;
}
    
    static Config parseConfigString(
        const std::string& ConfigStr,
        const std::vector<std::string>& AllFeatureNames) {
        
        Config Config;
        
        // Initialize all features to false
        for (const auto& Name : AllFeatureNames) {
            Config.Features[Name] = false;
        }
        
        // Parse selected features (comma-separated)
        if (ConfigStr.empty()) {
            return Config;  // All features false
        }
        
        std::stringstream SS(ConfigStr);
        std::string FeatureName;
        
        while (std::getline(SS, FeatureName, ',')) {
            // Trim whitespace
            FeatureName.erase(0, FeatureName.find_first_not_of("( \t\r\n)"));
            FeatureName.erase(FeatureName.find_last_not_of("( \t\r\n)") + 1);
            
            if (!FeatureName.empty()) {
                // Mark this feature as selected
                if (Config.Features.contains(FeatureName)) {
                    Config.Features[FeatureName] = true;
                } else {
                    std::cerr << "Warning: Unknown feature in paper: " << FeatureName << "\n";
                }
            }
        }
        
        return Config;
    }
    
    // Generate configs from your BDD
    std::set<Config> generateBDDConfigs(
    const vara::feature::FeatureModel& FeatureModel,
    oxidd::bdd_function& bdd,
    oxidd::bdd_manager& manager,
    bdd::sample::BDDFactory& factory,
    const std::vector<std::string>& FeatureNames,  
    bool HasSyntheticRoot) {
    
    std::set<Config> Configs;
    
    // Get expected count
    bdd::sample::BDDFactory Factory;
    auto Bdd = Factory.modelToBdd(FeatureModel);
    auto Manager = Bdd.containing_manager();
    double ExpectedCount = bdd.sat_count_double(Manager.num_vars());
    
    std::cout << "Expected configs from BDD: " << ExpectedCount << "\n";
    std::cout << "Generating configs...\n";
    
    // Generate until we find all unique ones
    int Attempts = 0;
    int MaxAttempts = std::max((int)ExpectedCount * 100, 10000);
    int NoNewCounter = 0;
    
    while (static_cast<double>(Configs.size()) < ExpectedCount && Attempts < MaxAttempts) {
        auto ConfigVec = bdd::sample::generateConfiguration(
            manager, bdd, factory, &factory.SatMap);
        
        // Convert to Config struct
        Config Config;
        
        size_t ModelIdx = 0;
        for (size_t I = 0; I < FeatureNames.size(); ++I) {
            // If there's a synthetic root, skip index 0 in configVec
            if (HasSyntheticRoot) {
                ModelIdx = I + 1;  // Skip first element (synthetic root)
            } else {
                ModelIdx = I;
            }
            
            if (ModelIdx < ConfigVec.size()) {
                Config.Features[FeatureNames[I]] = ConfigVec[ModelIdx];
            }
        }
        
        size_t Before = Configs.size();
        Configs.insert(Config);
        
        if (Configs.size() == Before) {
            NoNewCounter++;
            if (NoNewCounter > std::max((int)ExpectedCount * 10, 1000)) {
                std::cout << "\nStopping after " << Attempts << " attempts\n";
                break;
            }
        } else {
            NoNewCounter = 0;
        }
        
        Attempts++;
        
        if (Attempts % 1000 == 0) {
            std::cout << "  Attempts: " << Attempts 
                      << ", Found: " << Configs.size() 
                      << " / " << ExpectedCount << "\r" << std::flush;
        }
    }
    
    std::cout << "\nGenerated " << Configs.size() << " configs from BDD\n";
    return Configs;
}
    
    // Compare the two sets
    void compare(const std::set<Config>& PaperConfigs, 
                 const std::set<Config>& BddConfigs) {
        
        std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║              CONFIGURATION COMPARISON                    ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
        
        std::cout << "Paper configs: " << PaperConfigs.size() << "\n";
        std::cout << "BDD configs:   " << BddConfigs.size() << "\n";
        std::cout << "Difference:    " << ((int)PaperConfigs.size() - (int)BddConfigs.size()) << "\n\n";
        
        // Find configs in paper but not in BDD
        std::set<Config> OnlyInPaper;
        std::set_difference(
            PaperConfigs.begin(), PaperConfigs.end(),
            BddConfigs.begin(), BddConfigs.end(),
            std::inserter(OnlyInPaper, OnlyInPaper.begin())
        );
        
        // Find configs in BDD but not in paper
        std::set<Config> OnlyInBDD;
        std::set_difference(
            BddConfigs.begin(), BddConfigs.end(),
            PaperConfigs.begin(), PaperConfigs.end(),
            std::inserter(OnlyInBDD, OnlyInBDD.begin())
        );
        
        // Find configs in both
        std::set<Config> InBoth;
        std::set_intersection(
            PaperConfigs.begin(), PaperConfigs.end(),
            BddConfigs.begin(), BddConfigs.end(),
            std::inserter(InBoth, InBoth.begin())
        );
        
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "SUMMARY\n";
        std::cout << "═══════════════════════════════════════════════════════════\n\n";
        
        std::cout << "Configs in both:           " << InBoth.size() << "\n";
        std::cout << "Only in paper:             " << OnlyInPaper.size() << "\n";
        std::cout << "Only in BDD (yours):       " << OnlyInBDD.size() << "\n\n";
        
        if (OnlyInPaper.empty() && OnlyInBDD.empty()) {
            std::cout << "✓ PERFECT MATCH! All configs are identical.\n\n";
            return;
        }
        
        // Print differences
        if (!OnlyInPaper.empty()) {
            std::cout << "═══════════════════════════════════════════════════════════\n";
            std::cout << "CONFIGS IN PAPER BUT NOT IN YOUR BDD (" << OnlyInPaper.size() << ")\n";
            std::cout << "═══════════════════════════════════════════════════════════\n\n";
            
            int Count = 0;
            for (const auto& Config : OnlyInPaper) {
                std::cout << "  " << (++Count) << ". Selected: " << Config.toSelectedOnly() << "\n";
                if (Count >= 20 && OnlyInPaper.size() > 20) {
                    std::cout << "  ... and " << (OnlyInPaper.size() - 20) << " more\n";
                    break;
                }
            }
            std::cout << "\n";
        }
        
        if (!OnlyInBDD.empty()) {
            std::cout << "═══════════════════════════════════════════════════════════\n";
            std::cout << "CONFIGS IN YOUR BDD BUT NOT IN PAPER (" << OnlyInBDD.size() << ")\n";
            std::cout << "═══════════════════════════════════════════════════════════\n\n";
            
            int Count = 0;
            for (const auto& Config : OnlyInBDD) {
                std::cout << "  " << (++Count) << ". Selected: " << Config.toSelectedOnly() << "\n";
                if (Count >= 20 && OnlyInBDD.size() > 20) {
                    std::cout << "  ... and " << (OnlyInBDD.size() - 20) << " more\n";
                    break;
                }
            }
            std::cout << "\n";
        }
        
        // Analyze patterns
        analyzePatterns(OnlyInPaper, OnlyInBDD);
        
        // Export differences to CSV
        exportDifferences(OnlyInPaper, OnlyInBDD, "config_differences.csv");
    }
    
    static void analyzePatterns(
        const std::set<Config>& OnlyInPaper,
        const std::set<Config>& OnlyInBDD) {
        
        if (OnlyInPaper.empty() && OnlyInBDD.empty()) { 
            return;
        }
        
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "PATTERN ANALYSIS\n";
        std::cout << "═══════════════════════════════════════════════════════════\n\n";
        
        // Find features that differ
        std::map<std::string, int> PaperHasTrue;
        std::map<std::string, int> BddHasTrue;
        
        for (const auto& Config : OnlyInPaper) {
            for (const auto& [feature, value] : Config.Features) {
                if (value) { 
                    PaperHasTrue[feature]++;
                }
            }
        }
        
        for (const auto& Config : OnlyInBDD) {
            for (const auto& [feature, value] : Config.Features) {
                if (value) { 
                    BddHasTrue[feature]++;
                }
            }
        }
        
        std::cout << "Features frequently selected in paper configs:\n";
        for (const auto& [feature, count] : PaperHasTrue) {
            if (count > static_cast<double>(OnlyInPaper.size()) * 0.5) {
                std::cout << "  - " << feature << " (in " << count << "/" 
                          << OnlyInPaper.size() << " configs)\n";
            }
        }
        std::cout << "\n";
    }
    
    static void exportDifferences(
        const std::set<Config>& OnlyInPaper,
        const std::set<Config>& OnlyInBDD,
        const std::string& Filename) {
        
        std::ofstream Out(Filename);
        
        // Get all feature names
        std::set<std::string> AllFeatures;
        for (const auto& Config : OnlyInPaper) {
            for (const auto& [name, _] : Config.Features) {
                AllFeatures.insert(name);
            }
        }
        for (const auto& Config : OnlyInBDD) {
            for (const auto& [name, _] : Config.Features) {
                AllFeatures.insert(name);
            }
        }
        
        // Write header
        Out << "Source";
        for (const auto& Feature : AllFeatures) {
            Out << "," << Feature;
        }
        Out << "\n";
        
        // Write paper configs
        for (const auto& Config : OnlyInPaper) {
            Out << "Paper";
            for (const auto& Feature : AllFeatures) {
                auto It = Config.Features.find(Feature);
                Out << "," << (It != Config.Features.end() && It->second ? "1" : "0");
            }
            Out << "\n";
        }
        
        // Write BDD configs
        for (const auto& Config : OnlyInBDD) {
            Out << "BDD";
            for (const auto& Feature : AllFeatures) {
                auto It = Config.Features.find(Feature);
                Out << "," << (It != Config.Features.end() && It->second ? "1" : "0");
            }
            Out << "\n";
        }
        
        std::cout << "Differences exported to: " << Filename << "\n";
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

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <feature_model.xml> <paper_measurements.xml>\n";
        std::cerr << "Example: " << argv[0] << " LLVM.xml LLVM_measurements.xml\n";
        return 1;
    }
    
    std::string ModelPath = argv[1];
    std::string PaperMeasurementsPath = argv[2];
    
    ConfigComparator Comparator;
    
    // Load feature model first to get all feature names
    std::cout << "Loading feature model from: " << ModelPath << "\n";
    auto FeatureModel = [&]() {
        // Read the file content
        std::ifstream FileIn(ModelPath);
        if (!FileIn) {
            throw std::runtime_error("\033[31m Could not open file: \033[0m" + ModelPath);
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

    const auto *ConstModel = FeatureModel.get(); 

    size_t XmlFeatureCount = countFeaturesInXML(ModelPath);
    size_t ModelFeatureCount = ConstModel->size();
    bool HasSyntheticRoot = (ModelFeatureCount > XmlFeatureCount);
    
    std::vector<std::string> AllFeatureNames;
    for (auto* Feature : ConstModel->features()) {
        std::string Name = Feature->getName().str();

        if (HasSyntheticRoot && Name == "root" && !Feature->getParent()) {
            std::cout << "  Skipping synthetic root feature\n";
            continue;
        }
        
        AllFeatureNames.push_back(Name);
    }
    
    std::cout << "Feature model has " << AllFeatureNames.size() << " features\n\n";
    
    // Load paper configs
    std::cout << "Loading paper configs from: " << PaperMeasurementsPath << "\n";
    auto PaperConfigs = Comparator.loadPaperConfigs(PaperMeasurementsPath, AllFeatureNames);
    
    // Generate BDD configs
    std::cout << "\nGenerating configs from BDD...\n";
    bdd::sample::BDDFactory Factory;
    auto Bdd = Factory.modelToBdd(*FeatureModel);
    auto Manager = Bdd.containing_manager();
    
    auto BddConfigs = Comparator.generateBDDConfigs(
        *FeatureModel, Bdd, Manager, Factory, AllFeatureNames, HasSyntheticRoot);

    Comparator.compare(PaperConfigs, BddConfigs);
    
    return 0;
}
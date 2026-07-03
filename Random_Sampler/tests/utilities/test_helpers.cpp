#include "test_helpers.h"
#include "CoverageIntegration.h"
#include "CoverageMetrics.h"
#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModelParser.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace test_utils {

    class TestableCoverageEvaluator : public coverage::CoverageEvaluator {
public:
    using CoverageEvaluator::CoverageEvaluator;
    using CoverageEvaluator::computeMandatoryFeatures;
    using CoverageEvaluator::computeDeadFeatures;
    using CoverageEvaluator::getParentChildInteraction;
    using CoverageEvaluator::buildFeatureVarMap;
    using CoverageEvaluator::initializeMetrics;
};

std::vector<std::vector<bool>> loadConfigsFromCSV(
    const std::string& Filename,
    const vara::feature::FeatureModel& Fm) {
    
    std::ifstream In(Filename);
    if (!In) {
        throw std::runtime_error("Cannot open file: " + Filename);
    }
    
    std::string Line;
    std::vector<std::vector<bool>> Configs;
    
    // Skip header
    std::getline(In, Line);
    
    while (std::getline(In, Line)) {
        std::vector<bool> Config;
        std::stringstream Ss(Line);
        std::string Cell;
        
        // Skip ConfigID
        std::getline(Ss, Cell, ',');
        
        // Read values
        while (std::getline(Ss, Cell, ',')) {
            Config.push_back(Cell == "1");
        }
        
        Configs.push_back(Config);
    }
    
    In.close();
    return Configs;
}

void saveConfigsToCSV(
    const std::vector<std::vector<bool>>& Configs,
    const vara::feature::FeatureModel& Fm,
    const std::string& Filename) {
    
    std::ofstream Out(Filename);
    
    // Header
    Out << "ConfigID";
    for (auto* Feature : Fm.features()) {
        Out << "," << Feature->getName().str();
    }
    Out << "\n";
    
    // Data
    for (size_t I = 0; I < Configs.size(); ++I) {
        Out << I;
        for (bool Val : Configs[I]) {
            Out << "," << (Val ? "1" : "0");
        }
        Out << "\n";
    }
    
    Out.close();
}

std::map<std::string, double> loadExpectedCoverage(
    const std::string& Filename) {
    
    std::map<std::string, double> Results;
    std::ifstream In(Filename);
    
    if (!In) {
        throw std::runtime_error("Cannot open file: " + Filename);
    }
    
    std::string Line;
    std::getline(In, Line); // Skip header
    
    while (std::getline(In, Line)) {
        std::stringstream Ss(Line);
        std::string Metric;
        double Coverage;
        
        std::getline(Ss, Metric, ',');
        Ss >> Coverage;
        
        Results[Metric] = Coverage;
    }
    
    In.close();
    return Results;
}

void saveExpectedCoverage(
    const std::map<std::string, double>& Results,
    const std::string& Filename) {
    
    std::ofstream Out(Filename);
    
    Out << "Metric,Coverage\n";
    for (const auto& [metric, coverage] : Results) {
        Out << metric << "," << std::fixed << std::setprecision(6) 
            << coverage << "\n";
    }
    
    Out.close();
}

void printConfiguration(
    const std::vector<bool>& Config,
    const vara::feature::FeatureModel& Fm,
    bool OnlySelected) {
    
    size_t Idx = 0;
    for (auto* Feature : Fm.features()) {
        if (Idx >= Config.size()) { 
            break;
        }
        
        if (OnlySelected && !Config[Idx]) {
            ++Idx;
            continue;
        }
        
        ++Idx;
    }
    std::cout << "\n";
}

bool compareCoverageResults(
    const std::map<std::string, double>& Result1,
    const std::map<std::string, double>& Result2,
    double Tolerance) {
    
    if (Result1.size() != Result2.size()) {
        return false;
    }
    
    for (const auto& [metric, coverage1] : Result1) {
        auto It = Result2.find(metric);
        if (It == Result2.end()) {
            return false;
        }
        
        double Coverage2 = It->second;
        if (std::abs(coverage1 - Coverage2) > Tolerance) {
            return false;
        }
    }
    
    return true;
}

std::unique_ptr<vara::feature::FeatureModel> loadFeatureModel(
    const std::string& XmlPath) {
    
    std::ifstream FileIn(XmlPath);
    if (!FileIn) {
        throw std::runtime_error("Cannot open file: " + XmlPath);
    }
    
    std::ostringstream Oss;
    Oss << FileIn.rdbuf();
    
    vara::feature::FeatureModelXmlParser Parser(Oss.str());
    if (!Parser.verifyFeatureModel()) {
        throw std::runtime_error("Invalid feature model: " + XmlPath);
    }
    
    return Parser.buildFeatureModel();
}

std::unique_ptr<vara::feature::FeatureModel> createModelFromString(const std::string& Xml) {
    vara::feature::FeatureModelXmlParser Parser(Xml);
    if (!Parser.verifyFeatureModel()) {
        throw std::runtime_error("Invalid XML model");
    }
    return Parser.buildFeatureModel();
}

coverage::Interaction makeInteraction(std::vector<std::pair<std::string, bool>> const &Literals) {
    coverage::Interaction Interaction;
    for (const auto& [feature, value] : Literals) {
        Interaction.Literals[feature] = value;
    }
    return Interaction;
}

std::vector<bool> makeConfig(std::vector<bool> Values) {
    return Values;
}

} // namespace test_utils
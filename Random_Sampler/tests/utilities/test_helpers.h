#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "CoverageMetrics.h"
#include "vara/Feature/FeatureModel.h"
#include <map>
#include <string>
#include <vector>

namespace test_utils {

// Load configurations from CSV
std::vector<std::vector<bool>> loadConfigsFromCSV(
    const std::string& Filename,
    const vara::feature::FeatureModel& Fm);

// Save configurations to CSV
void saveConfigsToCSV(
    const std::vector<std::vector<bool>>& Configs,
    const vara::feature::FeatureModel& Fm,
    const std::string& Filename);

// Load expected coverage results
std::map<std::string, double> loadExpectedCoverage(
    const std::string& Filename);

// Save expected coverage results
void saveExpectedCoverage(
    const std::map<std::string, double>& Results,
    const std::string& Filename);

// Print configuration in readable format
void printConfiguration(
    const std::vector<bool>& Config,
    const vara::feature::FeatureModel& Fm,
    bool OnlySelected = false);

// Compare two coverage result maps
bool compareCoverageResults(
    const std::map<std::string, double>& Result1,
    const std::map<std::string, double>& Result2,
    double Tolerance = 0.001);

// Load feature model from file
std::unique_ptr<vara::feature::FeatureModel> loadFeatureModel(
    const std::string& XmlPath);

std::unique_ptr<vara::feature::FeatureModel> createModelFromString(const std::string& Xml);

coverage::Interaction makeInteraction(std::vector<std::pair<std::string, bool>> const &Literals);

std::vector<bool> makeConfig(std::vector<bool> Values);

} // namespace test_utils

#endif // TEST_HELPERS_H
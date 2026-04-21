#pragma once

#include "CoverageMetrics.h"

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <oxidd/bdd.hpp>

namespace vara::feature {
class FeatureModel;
} // namespace vara::feature

namespace twise {

using Configuration = std::vector<bool>;
using CandidateId = size_t;
using CoveredIdsList = std::vector<std::vector<CandidateId>>;

struct FeatureMaps {
    std::map<std::string, size_t> NameToIndex;
    std::map<size_t, std::string> IndexToName;
};

FeatureMaps buildFeatureMaps(const vara::feature::FeatureModel& FeatureModel);

std::map<std::string, oxidd::bdd_function> buildFeatureVarMap(
    const vara::feature::FeatureModel& FeatureModel,
    oxidd::bdd_manager& Manager
);

std::vector<Configuration> enumerateAllConfigurations(
    const oxidd::bdd_manager& Manager,
    const oxidd::bdd_function& Root
);

std::vector<Configuration> loadOrEnumerateConfigurations(
    const std::string& SystemName,
    const FeatureMaps& FeatureMap,
    const oxidd::bdd_manager& Manager,
    const oxidd::bdd_function& FinalBDD
);

std::set<coverage::Interaction> generateValidInteractions(
    size_t T,
    const vara::feature::FeatureModel& FeatureModel,
    const oxidd::bdd_function& BDD,
    const std::map<std::string, oxidd::bdd_function>& FeatureToBddVar
);

bool configurationCoversInteraction(
    const Configuration& Config,
    const coverage::Interaction& Interaction,
    const FeatureMaps& FeatureMap
);

CoveredIdsList precomputeCoveredIdsPerConfig(
    const std::vector<Configuration>& AllConfigs,
    const std::vector<coverage::Interaction>& CandidateList,
    const FeatureMaps& FeatureMap
);

CoveredIdsList loadOrPrecomputeCoveredIdsPerConfig(
    const std::string& SystemName,
    size_t T,
    const std::vector<Configuration>& AllConfigs,
    const std::vector<coverage::Interaction>& CandidateList,
    const FeatureMaps& FeatureMap
);

std::vector<size_t> greedyTWiseSamplingWithIds(
    const std::vector<Configuration>& AllConfigs,
    const CoveredIdsList& CoveredIdsPerConfig,
    size_t NumCandidates
);

size_t computeGreedySampleSize(
    const std::string& FilePath,
    size_t SourceT
);

std::string baseNameWithoutExtension(const std::string& Path);

void appendResultsToCsv(
    const std::string& OutputCsv,
    const std::string& SystemName,
    size_t SourceT,
    size_t SampleSize
);

} // namespace twise
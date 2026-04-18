#include "BDDFactory.h"
#include "CoverageMetrics.h"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "TWiseSampling.h"

std::string baseNameWithoutExtension(const std::string& Path) {
    size_t SlashPos = Path.find_last_of("/\\");
    std::string FileName =
        (SlashPos == std::string::npos) ? Path : Path.substr(SlashPos + 1);

    size_t DotPos = FileName.find_last_of('.');
    if (DotPos == std::string::npos) {
        return FileName;
    }
    return FileName.substr(0, DotPos);
}

void appendResultsToCsv(
    const std::string& OutputCsv,
    const std::string& SystemName,
    size_t SampleSize
) {
    bool WriteHeader = false;
    {
        std::ifstream In(OutputCsv);
        WriteHeader =
            !In.good() || In.peek() == std::ifstream::traits_type::eof();
    }

    std::ofstream Out(OutputCsv, std::ios::app);
    if (!Out) {
        throw std::runtime_error("Could not open output CSV: " + OutputCsv);
    }

    if (WriteHeader) {
        Out << "system,sample_size\n";
    }

    Out << SystemName << "," << SampleSize << "\n";
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 4) {
            std::cerr
                << "Usage: ./greedy_twise_sampling <feature_model.xml> <t> <output.csv>\n";
            return 1;
        }

        std::string FilePath = argv[1];
        size_t SourceT = std::stoul(argv[2]);
        std::string OutputCsv = argv[3];

        auto FeatureModel = vara::feature::loadFeatureModel(FilePath);

        bdd::sample::BDDFactory Factory;
        oxidd::bdd_function FinalBDD = Factory.modelToBdd(*FeatureModel);
        oxidd::bdd_manager Manager = FinalBDD.containing_manager();

        auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, Manager);
        twise::FeatureMaps FeatureMap = twise::buildFeatureMaps(*FeatureModel);

        std::set<coverage::Interaction> Candidates = twise::generateValidInteractions(
            SourceT,
            *FeatureModel,
            FinalBDD,
            FeatureToBddVar
        );

        std::vector<twise::Configuration> AllConfigs =
            twise::enumerateAllConfigurations(Manager, FinalBDD);

        std::cout << "Enumerated " << AllConfigs.size()
                  << " valid configurations.\n";

        std::vector<coverage::Interaction> CandidateList(
            Candidates.begin(),
            Candidates.end()
        );

        twise::CoveredIdsList CoveredIdsPerConfig = precomputeCoveredIdsPerConfig(
            AllConfigs,
            CandidateList,
            FeatureMap
        );

        std::string SystemName = baseNameWithoutExtension(FilePath);

        std::vector<size_t> Sample = twise::greedyTWiseSamplingWithIds(
            AllConfigs,
            CoveredIdsPerConfig,
            CandidateList.size()
        );

        appendResultsToCsv(OutputCsv, SystemName, Sample.size());

        return 0;
    } catch (const std::exception& E) {
        std::cerr << "ERROR: " << E.what() << "\n";
        return 1;
    }
}
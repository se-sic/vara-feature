#include "BDDSampler.h"
#include "../../BDD/include/BDDFactory.h"
#include "CoverageIntegration.h"
#include "vara/Feature/FeatureModelParser.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

extern "C" {
#include <oxidd/bdd.hpp>
}

using std::string;
using std::vector;

namespace {

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

std::string baseNameWithoutExtension(const std::string& Path) {
    size_t SlashPos = Path.find_last_of("/\\");
    std::string FileName = (SlashPos == std::string::npos) ? Path : Path.substr(SlashPos + 1);

    size_t DotPos = FileName.find_last_of('.');
    if (DotPos == std::string::npos) {
        return FileName;
    }
    return FileName.substr(0, DotPos);
}

void appendResultsToCsv(
    const std::string& OutputCsv,
    const std::string& SystemName,
    const std::string& Strategy,
    size_t SampleSizeSourceT,
    size_t SampleSize,
    size_t CoverageT,
    size_t Run,
    const std::map<std::string, double>& Results
) {
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
        Out << "system,strategy,sample_size_source_t,sample_size,coverage_t,run,metric,coverage\n";
    }

    for (const auto& [MetricName, Coverage] : Results) {
        Out << SystemName << ","
            << Strategy << ","
            << SampleSizeSourceT << ","
            << SampleSize << ","
            << CoverageT << ","
            << Run << ","
            << MetricName << ","
            << std::setprecision(17) << Coverage << "\n";
    }
}

std::vector<std::vector<bool>> generateRandomSample(
    oxidd::bdd_manager& Manager,
    const oxidd::bdd_function& Bdd,
    bdd::sample::BDDFactory& Factory,
    size_t SampleSize
) {
    std::vector<std::vector<bool>> AllSamples;
    AllSamples.reserve(SampleSize);

    for (size_t I = 0; I < SampleSize; ++I) {
        auto Config = bdd::sample::generateConfiguration(
            Manager,
            Bdd,
            Factory,
            &Factory.SatMap
        );
        AllSamples.push_back(std::move(Config));
    }

    return AllSamples;
}

} // namespace

int main(int argc, char* argv[]) noexcept(false) {
    try {
        if (argc < 7) {
            std::cerr << "Usage: ./experiment_runner <feature_model.xml> <strategy> "
                         "<sample_size_source_t> <sample_size> <run> <output.csv>\n";
            return 1;
        }

        std::string FilePath = argv[1];
        std::string Strategy = argv[2];
        size_t SampleSizeSourceT = std::stoul(argv[3]);
        size_t SampleSize = std::stoul(argv[4]);
        size_t Run = std::stoul(argv[5]);
        std::string OutputCsv = argv[6];

        auto FeatureModel = loadFeatureModel(FilePath);

        bdd::sample::BDDFactory Factory;
        oxidd::bdd_function FinalBDD = Factory.modelToBdd(*FeatureModel);
        oxidd::bdd_manager Manager = FinalBDD.containing_manager();

        std::vector<std::vector<bool>> Sample;

        if (Strategy == "random") {
            Sample = generateRandomSample(Manager, FinalBDD, Factory, SampleSize);
        } else {
            std::cerr << "Unsupported strategy for now: " << Strategy << "\n";
            return 2;
        }

        coverage::CoverageEvaluator Evaluator(*FeatureModel, FinalBDD, Manager, Factory);

        std::string SystemName = baseNameWithoutExtension(FilePath);

        for (size_t CoverageT : {1, 2, 3}) {
            auto Results = Evaluator.evaluateSample(Sample, CoverageT);

            appendResultsToCsv(
                OutputCsv,
                SystemName,        // Feature model name
                Strategy,          // Sampling strategy
                SampleSizeSourceT, // Source t defining the budget |S_t|
                SampleSize,        // Number of sampled configurations
                CoverageT,         // Coverage interaction strength
                Run,               // Repetition number
                Results            // Metric -> coverage map
            );
        }

        return 0;
    } catch (const std::exception& E) {
        std::cerr << "ERROR: " << E.what() << "\n";
        return 1;
    }
}
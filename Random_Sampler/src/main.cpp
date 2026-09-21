#include "BDDSampler.h"
#include "BDDFactory.h"
#include "EnumerateConfigs.h"
#include "EnumerateInteractions.h"
#include "InteractionCoverage.h"
#include "ParseFM.h"
#include "TWiseSampler.h"
#include "WriteFrequencies.h"
#include "WriteSamples.h"

#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelParser.h"

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

/// \brief BDD-based sampler for feature model configurations and interactions (t-wise and uniform random sampling).
///
/// Supported commands:
///     tsizes <t>                                                      --> enumerate all t-many interactions + reference sample sizes + generate t-wise sample (t = {1, 2, 3})
///     sample <strategy> <t>                                           --> generate a t-wise sample (same result as above) (strategy = "twise" or "random"; t = {1, 2, 3})
///     sample random <sample_size> <seed_value> <rq> [<proportion>]    --> generate a uniform random sample (sample_size = number of configurations to sample; seed_value = [1..100]; rq = {1, 2}; proportion = {0.01, 0.02, 0.05, 0.1, 0.15, 0.2})
///     sat_count                                                       --> count number of valid configurations of the feature model
///
/// Example: ./build/bin/random_sampler Random_Sampler/examples/Polly.xml tsizes 2

namespace {

    /// \brief Extracts the system name from the feature model file path, e.g. "Random_Sampler/examples/Polly.xml" -> "Polly"
    std::string extractSysName(const std::string &FMPath) {
        std::string Sys = FMPath;
        if (auto S = Sys.find_last_of("/ \\"); S != std::string::npos) {
            Sys = Sys.substr(S + 1);
        }
        if (auto D = Sys.find_last_of('.'); D != std::string::npos) {
            Sys = Sys.substr(0, D);
        }
        return Sys;
    }

    /// \brief Checks whether the every configuration from the \p Sample satisfies \p FinalBDD.
    bool validateAgainstBdd(const std::vector<std::vector<bool>> &Sample, const oxidd::bdd_function &FinalBDD) {
        for (const auto &Config : Sample) {
            std::vector<std::pair<oxidd::var_no_t, bool>> ValidTSamples;
                ValidTSamples.reserve(Config.size());
                for(oxidd::var_no_t V = 0; V < Config.size(); ++V) {
                    ValidTSamples.emplace_back(V, Config[V]);
                }
                if(!FinalBDD.eval(ValidTSamples)){
                    std::cerr << "Error: Generated an invalid configuration." << "\n";
                    return false;
                }
            }
        return true;
    }

    /// \brief Checks whether each interaction from \p Interactions is covered by at least one configuration in \p Sample.
    void validateAgainstCoverage(const std::vector<bdd::sample::Interaction> &Interactions, const std::vector<std::vector<bool>> &Sample) {
        for (const auto &I: Interactions) {
            bool CoveredInts = false;
            for (const auto &Config : Sample) {
                if(bdd::sample::CoverageCheck(Config, I)) {
                    CoveredInts = true; 
                    break;
                }
            }
            if(!CoveredInts) {
                std::cerr << "Error: Sample did not cover all interactions." << "\n";
            }
        }
    }

    /// \brief Parses an unsigned command-line argument
    bool parseCommand(const std::string &Cmd, unsigned &Out, const char *Label) {
        try {
            Out = static_cast<unsigned>(std::stoul(Cmd));
            return true;
        } catch (const std::invalid_argument &E) {
            std::cerr << "Error: Invalid value for " << Label << ": " << Cmd << '\n';
            return false;
        }
    }

    std::string tPathLabel(unsigned T) {
        return (T == 1 ? "T1" : (T ==2 ? "T2" : "T3")); //NOLINT Conditional operator is used as sub-expression of parent conditional operator, refrain from using nested conditional operator
    }

} //namespace

int main(int argc, char* argv[]) { 

    if (argc < 3) {
        std::cerr << "Error: Please follow argumnet structure: ./random_sampler <feature_model.xml> <t-sizes|sample|sat_count> [args...‚]\n";
        return 1;
    }

    const std::vector<std::string> Args(argv + 1, argv + argc);
    const std::string &FMPath = Args[0];
    const std::string &Command = Args[1];
    const std::string Sys = extractSysName(FMPath);

    // ------- Load feature model and build BDD -----------------------------------------------------------------------------------------------

    std::unique_ptr<vara::feature::FeatureModel> Fd = bdd::sample::ParseXML(FMPath);
    std::cerr << "Note: Feature Model loaded successfully from: " << FMPath <<'\n';

    bdd::sample::BDDFactory Factory;
    oxidd::bdd_function FinalBDD = Factory.modelToBdd(*Fd);
    oxidd::bdd_manager Manager = FinalBDD.containing_manager(); 
    std::cerr << "Note: BDD constructed successfully." << '\n';

   // ------- Enumerate all valid configurations -----------------------------------------------------------------------------------------------

    auto ValidCondfigs = bdd::sample::EnumerateConfigs(Manager, FinalBDD);
    const double Expected = FinalBDD.sat_count_double(Manager.num_vars());
    std::cerr << "Note: Valid Configs: " << ValidCondfigs.size() << " (expected " << Expected << ")\n";
    if (ValidCondfigs.size() != static_cast<std::size_t>(Expected)) {
        std::cerr << "Error: Enumeration count mismatch\n";
        return 1;
    }

    std::cerr << "Note: Enumerated all valid configurations successfully." << '\n';

    // ------- Command: tsizes -----------------------------------------------------------------------------------------------------------------

    if (Command == "tsizes") {
        unsigned T = 0;
        if (Args.size() < 3 || !parseCommand(Args[2], T, "t")) {
            return 1;
        }
        if (T < 1 || T > 3) {
            std::cerr << "Error: Value of t must be between 1 and 3 (inclusive). Provided: " << T << '\n';
            return 1;
        }

        auto Interactions = bdd::sample::EnumerateInteractions(Manager, T, ValidCondfigs);
        auto S = bdd::sample::TSample(ValidCondfigs, Interactions);

        if (!validateAgainstBdd(S, FinalBDD)) {
            return 1;
        }
        validateAgainstCoverage(Interactions, S);

        std::cout << S.size() << "\n";
        //std::string TFilePath = "bindings/python/Interplay_ML/Samples/sampler_test/" + Sys + "_TWiseSample.csv";
        std::string TFilePath = "bindings/python/Interplay_ML/Samples/" + tPathLabel(T) + "/" + Sys + "_TWiseSample.csv";
        std::filesystem::create_directories(std::filesystem::path(TFilePath).parent_path());
        bdd::sample::WriteSampleCSV(Manager, S, TFilePath);
        return 0;
    }

    /// ------- Command: sample -----------------------------------------------------------------------------------------------------------------

    if (Command == "sample") {
        if (Args.size() < 3) {
            std::cerr << "Error: Please provide a sampling strategy: 'twise' or 'random'.\n";
            return 1;
        }
        const std::string &Strat = Args[2];
        
        if (Strat == "twise") {
            // Arguments: "sample"[1] "twise"[2] <T>[3]
            unsigned T = 0;
            if (Args.size() < 4 || !parseCommand(Args[3], T, "t")) {
                return 1;
            }
            if (T < 1 || T > 3) {
                std::cerr << "Error: Value of t must be between 1 and 3 (inclusive). Provided: " << T << '\n';
                return 1;
            }

            auto Interactions = bdd::sample::EnumerateInteractions(Manager, T, ValidCondfigs);
            auto S   = bdd::sample::TSample(ValidCondfigs, Interactions);

            if (!validateAgainstBdd(S, FinalBDD)) {
                return 1;
            }
            validateAgainstCoverage(Interactions, S);

            //std::string TFilePath = "bindings/python/Interplay_ML/Samples/sampler_test/" + Sys + "_TWiseSample.csv";
            std::string TFilePath = "bindings/python/Interplay_ML/Samples/" + tPathLabel(T) + "/" + Sys + "_TWiseSample.csv";
            std::filesystem::create_directories(std::filesystem::path(TFilePath).parent_path());
            bdd::sample::WriteSampleCSV(Manager, S, TFilePath);
            return 0;
        } 
        
        if (Strat == "random") {
            // Arguments: "sample"[1] "random"[2] <T>[3] <sample_size>[4] <seed_value>[5] <rq>[6] [<proportion>[7]]
            if (Args.size() < 7) {
                std::cerr << "Error: Please provide T, sample size, seed, rq and optionally proportion." << '\n';
                return 1;
            }
            
            unsigned T = 0; 
            unsigned SampleSize = 0;
            unsigned Seed = 0;
            unsigned Rq = 1;

            if (Args[3] != "None" && Args[3] != "0" && !parseCommand(Args[3], T, "t")) {
                return 1;
            }
            if (!parseCommand(Args[4],SampleSize, "sample_size")) {
                return 1;
            }
            if (!parseCommand(Args[5], Seed, "seed_value")) {
                return 1;
            }
            if (!parseCommand(Args[6], Rq, "rq")) {
                return 1;
            }

            if ((T < 1 || T > 3) && Rq ==1) {
                std::cerr << "Error: Value of t must be between 1 and 3 (inclusive). Provided: " << T << '\n';
                return 1;
            }

            std::set<std::vector<bool>> Seen;
            bdd::sample::SeedRng(Seed);
            std::vector<std::vector<bool>> Samples;
            Samples.reserve(SampleSize);

            std::size_t MaxAttempts = SampleSize * 10000; //NOLINT Performing an implicit widening conversion to type 'std::size_t' (aka 'unsigned long') of a multiplication performed in type 'unsigned int'
            std::size_t Attempt = 0;
            while(Samples.size() < SampleSize && Attempt++ < MaxAttempts) {
                auto S = bdd::sample::generateConfiguration(
                    Manager, 
                    FinalBDD, 
                    &Factory.getSatMap()
                );
                
                if (Seen.insert(S).second) {
                    Samples.push_back(S);
                }
            }

            if (Samples.size() < SampleSize) {
                std::cerr << "Error: could only draw " << Samples.size() << " of expected " << SampleSize << " configgurations" << '\n';
                return 1;
            }
        
            std::string Prop = (Args.size() > 7) ? Args[7] : "NA";
            std::string FilePath;
            std::string RqPath = "RQ" + std::to_string(Rq);
            if (Rq == 2) {
                FilePath = "bindings/python/Interplay_ML/Samples/RQ2/" + Prop + "%/" + Sys +
                "/" + Sys + "_" + std::to_string(Seed) + ".csv";
            } else {
                //FilePath = "bindings/python/Interplay_ML/Samples/sampler_test/" + Sys + "_RandomSample.csv";
                FilePath = "bindings/python/Interplay_ML/Samples/" + RqPath + "/Random/" + Sys + "/" + tPathLabel(T) + "/" + Sys + "_" + std::to_string(Seed) + ".csv";
            }
            std::filesystem::create_directories(std::filesystem::path(FilePath).parent_path());
            if (Seed == 1) { bdd::sample::WriteFrequencyCSV(FinalBDD, Samples); }
            bdd::sample::WriteSampleCSV(Manager, Samples, FilePath);
            return 0;
        }

        std::cerr << "Error: Unknown sampling strategy " << Strat << ". Use 'twise' or 'random'" << '\n';
        return 1;
    }

    /// ------- Command: sat_count -----------------------------------------------------------------------------------------------------------------

    if (Command == "sat_count") {
        double Count = FinalBDD.sat_count_double(Manager.num_vars());
        std::cout << Count << "\n";
        return 0;
    }

    std::cerr << "Error: Unknown command " << Command << ". Use 'tsizes', 'sample' or 'sat_count'" << '\n';
    return 1;
}
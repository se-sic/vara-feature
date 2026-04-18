#include "CoverageIntegration.h"
#include "CoverageMetrics.h"
#include "oxidd/bdd.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>

namespace coverage {

CoverageEvaluator::CoverageEvaluator(
    const vara::feature::FeatureModel& FeatureModel,
    const oxidd::bdd_function& Bdd,
    oxidd::bdd_manager& Manager,
    bdd::sample::BDDFactory& Factory
) : FeatureModel(FeatureModel), BDD(Bdd), Manager(Manager), Factory(Factory) {
    
    buildFeatureVarMap();
    initializeAnalysis();
    initializeMetrics();
}

void CoverageEvaluator::initializeAnalysis() {
    std::cout << "Initializing feature model analysis...\n";
    
    // Extract basic feature information
    size_t Idx = 0;
    for (auto* Feature : FeatureModel.features()) {
        FeatureInfo Info;
        Info.Name = Feature->getName().str();
        Info.Index = Idx;
        
        Analysis.Features.push_back(Info);
        Analysis.NameToIndex[Info.Name] = Idx;
        Analysis.IndexToName[Idx] = Info.Name;
        
        ++Idx;
    }

    std::cout << "  Found " << Analysis.Features.size() << " features\n";
    std::cout << "  Idx " << Idx << " features\n";
    
    // Compute mandatory and dead features
    std::cout << "  Computing mandatory features...\n";
    Analysis.MandatoryFeatures = computeMandatoryFeatures();
    std::cout << "    Mandatory features: " << Analysis.MandatoryFeatures.size() << "\n";
    
    std::cout << "  Computing dead features...\n";
    Analysis.DeadFeatures = computeDeadFeatures();
    std::cout << "    Dead features: " << Analysis.DeadFeatures.size() << "\n";

    Analysis.ParentChildPairs = getParentChildInteraction();

    Analysis.AtomicLiteralSets = getAtomicLiteralSets();
}



void CoverageEvaluator::initializeMetrics() {
    // Add all metrics
    Metrics.push_back(MetricFactory::createMDAP()); // MF-DF-ALS-PCI
    Metrics.push_back(MetricFactory::createMDA()); // MF-DF-ALS
    Metrics.push_back(MetricFactory::createMDP()); // MF-DF-PCI
    Metrics.push_back(MetricFactory::createMD()); // MF-DF
    Metrics.push_back(MetricFactory::createAP()); // ALS-PCI
    Metrics.push_back(MetricFactory::createPCI()); // PCI
    Metrics.push_back(MetricFactory::createALS()); // ALS
    Metrics.push_back(MetricFactory::createDefault()); // Default
}

std::set<size_t> CoverageEvaluator::computeMandatoryFeatures() {
    std::set<size_t> MandatoryFeatures;

    std::cout << "Detecting core features (exact)...\n";

    for (auto* Feature : FeatureModel.features()) {
        try {
            const std::string FeatureName = Feature->getName().str();

            auto MaybeVar = Manager.name_to_var(Feature->getName());
            if (!MaybeVar) {
                std::cerr << "\033[33m"
                          << "Warning: No BDD variable found for feature "
                          << FeatureName << "\033[0m\n";
                continue;
            }

            auto Var = Manager.var(*MaybeVar);
            auto Constrained = BDD & ~Var;

            if (!Constrained.satisfiable()) {
                auto It = Analysis.NameToIndex.find(FeatureName);
                if (It != Analysis.NameToIndex.end()) {
                    MandatoryFeatures.insert(It->second);
                }
            }

        } catch (const std::exception& E) {
            std::cerr << "\033[31m"
                      << "Warning: Error checking feature "
                      << Feature->getName().str()
                      << " for core: " << E.what()
                      << "\033[0m\n";
            continue;
        }
    }

    std::cout << "Mandatory features: ";
    for (size_t Index : MandatoryFeatures) {
        auto NameIt = Analysis.IndexToName.find(Index);
        if (NameIt != Analysis.IndexToName.end()) {
            std::cout << NameIt->second << " ";
        }
    }
    std::cout << '\n';

    std::cout << "Found " << MandatoryFeatures.size() << " core features\n";
    return MandatoryFeatures;
}

std::set<size_t> CoverageEvaluator::computeDeadFeatures() {
    std::set<size_t> DeadFeatures;

    std::cout << "Detecting dead features (exact)...\n";

    for (auto* Feature : FeatureModel.features()) {
        try {
            const std::string FeatureName = Feature->getName().str();

            auto MaybeVar = Manager.name_to_var(Feature->getName());
            if (!MaybeVar) {
                std::cerr << "\033[33m"
                          << "Warning: No BDD variable found for feature "
                          << FeatureName << "\033[0m\n";
                continue;
            }

            auto Var = Manager.var(*MaybeVar);
            auto Constrained = BDD & Var;

            if (!Constrained.satisfiable()) {
                auto It = Analysis.NameToIndex.find(FeatureName);
                if (It != Analysis.NameToIndex.end()) {
                    DeadFeatures.insert(It->second);
                }
            }

        } catch (const std::exception& E) {
            std::cerr << "\033[31m"
                      << "Warning: Error checking feature "
                      << Feature->getName().str()
                      << " for dead: " << E.what()
                      << "\033[0m\n";
            continue;
        }
    }

    std::cout << "Found " << DeadFeatures.size() << " dead features\n";
    return DeadFeatures;
}

std::set<FeaturePair> CoverageEvaluator::getParentChildInteraction() {
    std::set<FeaturePair> ParentChildInteraction;

    for(auto *F: FeatureModel.features()) { 
        if(F->getParentFeature() == nullptr) {
            continue;
        }
        
        FeaturePair ParentChild{F, F->getParentFeature()};
        ParentChildInteraction.emplace(ParentChild);
    }
    
    return ParentChildInteraction;
}

oxidd::bdd_function CoverageEvaluator::makeLiteral(size_t Idx, bool Value) {
    auto NameIt = Analysis.IndexToName.find(Idx);
    if (NameIt == Analysis.IndexToName.end()) {
        throw std::runtime_error("No feature name for index " + std::to_string(Idx));
    }

    auto VarIt = FeatureToBddVar.find(NameIt->second);
    if (VarIt == FeatureToBddVar.end()) {
        throw std::runtime_error("No BDD var for feature " + NameIt->second);
    }

    return Value ? VarIt->second : ~VarIt->second;
}

bool CoverageEvaluator::implies(const LiteralKey& A, const LiteralKey& B) {
    auto LitA = makeLiteral(A.FeatureIdx, A.Value);
    auto LitB = makeLiteral(B.FeatureIdx, B.Value);

    auto Constrained = BDD & LitA & ~LitB;
    return !Constrained.is_invalid() && !Constrained.satisfiable();
}

std::map<LiteralKey, std::set<LiteralKey>> CoverageEvaluator::getAtomicLiteralSets() {
    std::vector<LiteralKey> Literals;
    for (size_t I = 0; I < FeatureModel.size(); ++I) {
        Literals.push_back({I, true});
        Literals.push_back({I, false});
    }

    std::set<LiteralKey> Visited;
    std::map<LiteralKey, std::set<LiteralKey>> Result;

    for (const auto& Lit : Literals) {
        if (Visited.contains(Lit)) {
            continue;
        }

        std::set<LiteralKey> Cls;
        Cls.insert(Lit);
        Visited.insert(Lit);

        for (const auto& Other : Literals) {
            if (Visited.contains(Other)) {
                continue;
            }

            if (implies(Lit, Other) && implies(Other, Lit)) {
                Cls.insert(Other);
                Visited.insert(Other);
            }
        }

        LiteralKey Representative = *Cls.begin();
        Result[Representative] = Cls;
    }

    return Result;
}

std::set<Interaction> CoverageEvaluator::generateValidInteractions(size_t T) {
    // Was it already computed and cached?
    if (ValidInteractionsCache.contains(T)) {
        return ValidInteractionsCache[T];
    }
    
    std::cout << "Generating all valid " << T << "-wise interactions using BDD...\n";
    
    std::set<Interaction> ValidInteractions;
    
    // Collects all feature names
    std::vector<std::string> FeatureNames;
    for (auto* Feature : FeatureModel.features()) {
        FeatureNames.push_back(Feature->getName().str());
    }
    size_t NumberOfFeatures = FeatureNames.size();
    
    // Calculate how many feature interactions exist
    size_t TotalCombination = binomialCoefficient(NumberOfFeatures, T);
    size_t Processed = 0;
    
    std::cout << "  Checking " << TotalCombination << " feature combinations...\n";
    
    // Generate all combinations of t features
    std::function<void(size_t, std::vector<size_t>&)> GenerateCombos;
    GenerateCombos = [&](size_t Start, std::vector<size_t>& Current) {
        if (Current.size() == T) {
            checkAllValueAssignments(Current, FeatureNames, ValidInteractions);
            
            ++Processed;
            if (Processed % 100 == 0) {
                std::cout << "  Progress: " << Processed << "/" << TotalCombination 
                          << " (" << ValidInteractions.size() << " valid)\r" 
                          << std::flush;
            }
            return;
        }
        
        // [0,1] [0,2] [0,3] [1,2] [1,3] [2,3]
        for (size_t I = Start; I < NumberOfFeatures; ++I) {
            Current.push_back(I);
            GenerateCombos(I + 1, Current);
            Current.pop_back();
        }
    };
    
    std::vector<size_t> Current;
    GenerateCombos(0, Current);
    
    std::cout << "\n"<< "\033[32m" << " Total: " << ValidInteractions.size() 
              << " unique valid interactions found \033[0m\n";
    
    // Cache result
    ValidInteractionsCache[T] = ValidInteractions;
    
    return ValidInteractions;
}

void CoverageEvaluator::checkAllValueAssignments(
    const std::vector<size_t>& FeatureIndices,
    const std::vector<std::string>& FeatureNames,
    std::set<Interaction>& ValidInteractions) {

    size_t T = FeatureIndices.size();

    // 1<<T = 2^n 0 = 000 all false  1 = 001 third feature true..
    for (size_t Mask = 0; Mask < (1<<T); ++Mask) {
        Interaction Interaction;

        for (size_t I = 0; I < T; ++I) {
            size_t FeatureIndex = FeatureIndices[I]; // FeatureIndex = 5
            bool Value = (Mask >> I) & 1; // (Mask >> 1) & 1 = 10 & 1 = 0 
            Interaction.Literals[FeatureNames[FeatureIndex]] = Value; // B false
        }

        if (isInteractionSatisfiable(Interaction)) {
            ValidInteractions.insert(Interaction);
        }
    }

}

void CoverageEvaluator::buildFeatureVarMap() {
    std::cout << "Building feature-to-BDD-variable map...\n";

    FeatureToBddVar.clear();

    // For each feature
    for (auto* Feature : FeatureModel.features()) {
        const std::string Name = Feature->getName().str();

        // Give me the corresponding variable
        auto MaybeVar = Manager.name_to_var(Feature->getName());
        if (!MaybeVar) {
            std::cerr << "\033[33m"
                      << "Warning: No BDD variable found for feature "
                      << Name << "\033[0m\n";
            continue;
        }

        auto Var = Manager.var(*MaybeVar);
        if (Var.is_invalid()) {
            std::cerr << "\033[31m"
                      << "Warning: Invalid BDD variable for feature "
                      << Name << "\033[0m\n";
            continue;
        }

        FeatureToBddVar[Name] = Var;
    }

    std::cout << "\033[32m"
              << "Mapped " << FeatureToBddVar.size()
              << " features to BDD variables\033[0m\n";
}

bool CoverageEvaluator::isInteractionSatisfiable(const Interaction& Interaction) {
    oxidd::bdd_function Constrained = BDD;

    if (Constrained.is_invalid()) {
        std::cerr << "Constrained BDD became invalid for interaction: { ";
        std::cerr << "}\n";
        return false;
    }

    for (const auto& [featureName, value] : Interaction.Literals) {
        auto It = FeatureToBddVar.find(featureName);
        if (It == FeatureToBddVar.end()) {
            std::cerr << "\033[31m"
                      << "WARNING: Feature '" << featureName
                      << "' not in BDD variable map\033[0m\n";
            return false;
        }

        oxidd::bdd_function Var = It->second;
        if (Var.is_invalid()) {
            std::cerr << "\033[31m"
                      << "ERROR: Invalid mapped BDD variable for feature "
                      << featureName << "\033[0m\n";
            return false;
        }

        // Turns it into positive or negative literal
        oxidd::bdd_function Literal = value ? Var : ~Var;
        if (Literal.is_invalid()) {
            std::cerr << "\033[31m"
                      << "ERROR: Invalid literal for feature "
                      << featureName << "\033[0m\n";
            return false;
        }

        Constrained = Constrained & Literal;
        if (Constrained.is_invalid()) {
            // std::cerr << "\033[31m"
            //           << "Constrained BDD became invalid after feature "
            //           << featureName << "\033[0m\n";
            return false;
        }

        if (!Constrained.satisfiable()) {
            return false;
        }
    }

    return Constrained.is_invalid() ? false : Constrained.satisfiable();
}

size_t CoverageEvaluator::binomialCoefficient(size_t N, size_t K) {
    if (K > N) {
        return 0;
    }

    if (K == 0 || K == N) {
        return 1;
    }

    K = std::min(K, N - K);

    size_t Result = 1;

    for (size_t I = 0; I < K; ++I) {
        Result = Result * (N - I) / (I + 1);
    }

    return Result;
}

std::set<Interaction> CoverageEvaluator::extractInteractionsFromConfig(
    const std::vector<bool>& Config,
    size_t T) {
    
    std::set<Interaction> Interactions;
    
    size_t NumFeatures = std::min(Config.size(), Analysis.Features.size());
    
    // Generate all t-wise combinations from this config
    std::function<void(size_t, std::vector<size_t>&)> GenerateCombos;
    GenerateCombos = [&](size_t Start, std::vector<size_t>& Current) {
        if (Current.size() == T) {
            Interaction Interaction;
            for (size_t Idx : Current) {
                if (Analysis.IndexToName.contains(Idx)) {
                    std::string Name = Analysis.IndexToName[Idx];
                    Interaction.Literals[Name] = Config[Idx];
                }
            }
            if (Interaction.Literals.size() == T) {
                Interactions.insert(Interaction);
            }
            return;
        }
        
        for (size_t I = Start; I < NumFeatures; ++I) {
            Current.push_back(I);
            GenerateCombos(I + 1, Current);
            Current.pop_back();
        }
    };
    
    std::vector<size_t> Current;
    GenerateCombos(0, Current);
    
    return Interactions;
}

std::map<std::string, double> CoverageEvaluator::evaluateSample(
    const std::vector<std::vector<bool>>& Sample,
    size_t T
) {
    std::map<std::string, double> Results;
    
    // Get or generate all t-wise valid interactions
    auto ValidInteractions = generateValidInteractions(T);
    
    std::cout << "\nEvaluating sample with " << Sample.size() 
              << " configurations...\n";
    
    // Evaluate each metric
    for (const auto& Metric : Metrics) {
        auto Start = std::chrono::high_resolution_clock::now();
        
        double Coverage = Metric.compute(ValidInteractions, Sample, Analysis, T);
        
        auto End = std::chrono::high_resolution_clock::now();
        auto Duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            End - Start).count();
        
        Results[Metric.getName()] = Coverage;
        
        std::cout << "\033[35m" << "  " << Metric.getName() << ": " 
                  << (Coverage * 100.0) << "% (computed in " 
                  << Duration << " ms)\033[0m\n";
    }
    
    return Results;
}

std::map<std::string, std::vector<double>> CoverageEvaluator::evaluateIncremental(
    const std::vector<std::vector<bool>>& Sample,
    size_t T,
    const std::vector<size_t>& Checkpoints
) {
    std::map<std::string, std::vector<double>> Results;
    
    // Initialize result vectors
    for (const auto& Metric : Metrics) {
        Results[Metric.getName()] = std::vector<double>();
    }
    
    // Get or generate valid interactions
    auto ValidInteractions = generateValidInteractions(T);

    std::cout << "\n Size of valid interactions" << ValidInteractions.size() << "\n";
    
    std::cout << "\nEvaluating incremental coverage...\n";
    
    // Evaluate at each checkpoint
    for (size_t Checkpoint : Checkpoints) {
        if (Checkpoint > Sample.size()) {
            std::cerr << "\033[31m" << "Warning: Checkpoint " << Checkpoint 
                    << " exceeds sample size " << Sample.size() << "\033[0m\n";
            continue;
        }
        
        std::cout << "\033[35m" << "  Checkpoint: " << Checkpoint << " configurations\033[0m\n";
        
        // Explicitly cast checkpoint to a signed type to avoid narrowing warning
        auto Offset = static_cast<std::ptrdiff_t>(Checkpoint);
        
        std::vector<std::vector<bool>> PartialSample(
            Sample.begin(),
            Sample.begin() + Offset
        );
        
        for (const auto& Metric : Metrics) {
            double Coverage = Metric.compute(ValidInteractions, PartialSample, Analysis, T);
            Results[Metric.getName()].push_back(Coverage);
        }
    }
    
    return Results;
}

  void CoverageEvaluator::exportToCSV(
    const std::string& Filename,
    const std::map<std::string, std::vector<double>>& Results,
    const std::vector<size_t>& SampleSizes
) {
    std::ofstream File(Filename);
    
    // Write header
    File << "SampleSize";
    for (const auto& [metricName, _] : Results) {
        File << "," << metricName;
    }
    File << "\n";
    
    // Write data
    for (size_t I = 0; I < SampleSizes.size(); ++I) {
        File << SampleSizes[I];
        for (const auto& [metricName, coverages] : Results) {
            if (I < coverages.size()) {
                File << "," << coverages[I];
            } else {
                File << ",";
            }
        }
        File << "\n";
    }
    
    File.close();
    std::cout << "Results exported to " << Filename << "\n";
}
} // namespace coverage

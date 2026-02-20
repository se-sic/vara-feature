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
    
    initializeAnalysis();
    buildFeatureVarMap();
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
    
    // Compute mandatory and dead features
    std::cout << "  Computing mandatory features...\n";
    Analysis.MandatoryFeatures = computeMandatoryFeatures();
    std::cout << "    Mandatory features: " << Analysis.MandatoryFeatures.size() << "\n";
    
    std::cout << "  Computing dead features...\n";
    Analysis.DeadFeatures = computeDeadFeatures();
    std::cout << "    Dead features: " << Analysis.DeadFeatures.size() << "\n";

    Analysis.ParentChildPairs = getParentChildInteraction();
}



void CoverageEvaluator::initializeMetrics() {
    // Add all metrics
    // metrics_.push_back(MetricFactory::createM1()); // MF-DF-ALS-PCI
    // metrics_.push_back(MetricFactory::createM2()); // MF-DF-ALS
    // metrics_.push_back(MetricFactory::createM3()); // MF-DF-PCI
    Metrics.push_back(MetricFactory::createM4()); // MF-DF
    Metrics.push_back(MetricFactory::createM5()); // PCI
    // metrics_.push_back(MetricFactory::createM6()); // ALS
    Metrics.push_back(MetricFactory::createM7()); // Default
}

std::set<size_t> CoverageEvaluator::computeMandatoryFeatures() {
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

std::set<size_t> CoverageEvaluator::computeDeadFeatures() {
    std::set<size_t> DeadFeatures;
    
    std::cout << "Detecting dead features (exact)...\n";
    
    // For each feature, check if it's never selected
    // A feature is dead if (BDD & feature) is unsatisfiable
    for (size_t I = 0; I < FeatureModel.size(); ++I) {
        try {
            // Get feature_i (positive variable)
            auto VarI = Manager.var(static_cast<oxidd::var_no_t>(I));
            
            // Compute: BDD & feature_i
            auto Constrained = BDD & VarI;
            
            // If unsatisfiable, feature must always be false (dead)
            if (!Constrained.satisfiable()) {
                DeadFeatures.insert(I);
            }
            
        } catch (const std::exception& E) {
            std::cerr << "\033[31m" << "Warning: Error checking feature " << I 
                      << " for dead: " << E.what() << "\033[0m\n";
            continue;
        }
    }
    
    std::cout << "    Found " << DeadFeatures.size() << " dead features\n";
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

std::set<Interaction> CoverageEvaluator::generateValidInteractions(size_t T) {
    // Check cache
    if (ValidInteractionsCache.contains(T)) {
        return ValidInteractionsCache[T];
    }
    
    std::cout << "Generating all valid " << T << "-wise interactions using BDD...\\n";
    
    std::set<Interaction> ValidInteractions;
    
    // Get feature names
    std::vector<std::string> FeatureNames;
    for (auto* Feature : FeatureModel.features()) {
        FeatureNames.push_back(Feature->getName().str());
    }
    size_t NumberOfFeatures = FeatureNames.size();
    
    // Calculate total combinations
    size_t TotalCombination = binomialCoefficient(NumberOfFeatures, T);
    size_t Processed = 0;
    
    std::cout << "  Checking " << TotalCombination << " feature combinations...\\n";
    
    // Generate all combinations of t features
    std::function<void(size_t, std::vector<size_t>&)> GenerateCombos;
    GenerateCombos = [&](size_t Start, std::vector<size_t>& Current) {
        if (Current.size() == T) {
            checkAllValueAssignments(Current, FeatureNames, ValidInteractions);
            
            ++Processed;
            if (Processed % 100 == 0) {
                std::cout << "  Progress: " << Processed << "/" << TotalCombination 
                          << " (" << ValidInteractions.size() << " valid)\\r" 
                          << std::flush;
            }
            return;
        }
        
        for (size_t I = Start; I < NumberOfFeatures; ++I) {
            Current.push_back(I);
            GenerateCombos(I + 1, Current);
            Current.pop_back();
        }
    };
    
    std::vector<size_t> Current;
    GenerateCombos(0, Current);
    
    std::cout << "\\n"<< "\033[32m" << " Total: " << ValidInteractions.size() 
              << " unique valid interactions found \033[0m\\n";
    
    // Cache result
    ValidInteractionsCache[T] = ValidInteractions;
    
    return ValidInteractions;
}

void CoverageEvaluator::checkAllValueAssignments(
    const std::vector<size_t>& FeatureIndices,
    const std::vector<std::string>& FeatureNames,
    std::set<Interaction>& ValidInteractions) {

    size_t T = FeatureIndices.size();

    for (size_t Mask = 0; Mask < (1<<T); ++Mask) {
        Interaction Interaction;

        for (size_t I = 0; I < T; ++I) {
            size_t FeatureIndex = FeatureIndices[I];
            bool Value = (Mask >> I) & 1;
            Interaction.Literals[FeatureNames[FeatureIndex]] = Value;
        }

        if (isInteractionSatisfiable(Interaction)) {
            ValidInteractions.insert(Interaction);
        }
    }

}

void CoverageEvaluator::buildFeatureVarMap() {
    std::cout << "Building feature-to-BDD-variable map...\\n";
    
    size_t Idx = 0;
    for (auto* Feature : FeatureModel.features()) {
        std::string Name = Feature->getName().str();
        
        if (Idx < Factory.Vars.size()) {
            FeatureToBddVar[Name] = Factory.Vars[Idx];
        } else {
            std::cerr << "\033[31m" << "ERROR: Not enough variables in factory!\033[0m\\n";
        }
        
        ++Idx;
    }
    
    std::cout << "\033[32m" << "Mapped " << FeatureToBddVar.size() 
              << " features to BDD variables\033[0m\\n";
}

bool CoverageEvaluator::isInteractionSatisfiable(const Interaction& Interaction) {
    // Start with the full model BDD (already reduced!)
    oxidd::bdd_function Constrained = BDD;
    
    // Apply each literal as a constraint
    for (const auto& [featureName, value] : Interaction.Literals) {
        // Look up the BDD variable in our map
        auto It = FeatureToBddVar.find(featureName);
        if (It == FeatureToBddVar.end()) {
            std::cerr << "\033[31m" << "WARNING: Feature '" << featureName 
                      << "' not in BDD variable map\033[0m\\n";
            continue;
        }
        
        // Get the BDD variable
        oxidd::bdd_function Var = It->second;
        
        // Create literal: var if value=true, ~var if value=false
        oxidd::bdd_function Literal = value ? Var : ~Var;
        
        // AND with the constraints
        Constrained = Constrained & Literal;
        
        // Early termination: if already unsatisfiable, stop
        if (!Constrained.satisfiable()) {
            return false;
        }
    }
    
    // Check if the final constrained BDD is satisfiable
    return Constrained.satisfiable();
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
    
    // Get or generate valid interactions
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
                  << Duration << " ms)\n";
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

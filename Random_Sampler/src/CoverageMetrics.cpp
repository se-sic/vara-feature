#include "CoverageMetrics.h"
#include <cmath>

namespace coverage {

// Coverage metric implementation

double CoverageMetric::compute(const std::set<Interaction>& AllValidInteractions,
                               const std::vector<Configuration>& Sample,
                               const FeatureModelAnalysis& Analysis,
                               size_t T) const {
    // Applies filter to get the set of interactions to consider
    std::set<Interaction> FilteredInteractions = Filter(AllValidInteractions, Analysis);
    
    // if |∇(♢(M,t))| = 0
    if (FilteredInteractions.empty()) {
        return 1.0;
    }
    
    // Extracts covered interactions from the sample {i ∈ ∇(♢(M,t)) | ∃γ ∈ S : i ∈ ⟦γ⟧}
    std::set<Interaction> CoveredInteractions = extractCoveredInteractions(Sample, Analysis, T);
    
    // Counts how many filtered interactions are covered
    size_t CoveredCount = 0;
    for (const auto& Interaction : FilteredInteractions) { // For each i ∈ ∇(♢(M,t))
        if (CoveredInteractions.contains(Interaction)) { // Check if i ∈ coveredInteractions
            ++CoveredCount;
        }
    }
    
    // Returns coverage ratio Cov = |{i ∈ ∇(♢(M,t)) | ∃γ ∈ S : i ∈ ⟦γ⟧}| / |∇(♢(M,t))|
    return static_cast<double>(CoveredCount) / static_cast<double>(FilteredInteractions.size());
}


// Filter implementations

std::set<Interaction> filterDefault(const std::set<Interaction>& Interactions,
                                   const FeatureModelAnalysis& Analysis) {
    // Identity function - no filtering
    return Interactions;
}

std::set<Interaction> filterMandatoryFeatures(const std::set<Interaction>& Interactions,
                                        const FeatureModelAnalysis& Analysis) {
    std::set<Interaction> Filtered;
    
    for (const auto& Interaction : Interactions) {
        bool ContainsMandatory = false;
        
        // Check if interaction contains any mandatory feature
        for (const auto& [featureName, selected] : Interaction.Literals) {
            size_t FeatureIdx = Analysis.NameToIndex.at(featureName);
            if (Analysis.MandatoryFeatures.contains(FeatureIdx) && selected) {
                ContainsMandatory = true;
                break;
            }
        }
        
        if (!ContainsMandatory) {
            Filtered.insert(Interaction);
        }
    }
    
    return Filtered;
}

std::set<Interaction> filterDeadFeatures(const std::set<Interaction>& Interactions,
                                        const FeatureModelAnalysis& Analysis) {
    std::set<Interaction> Filtered;
    
    for (const auto& Interaction : Interactions) {
        bool ContainsDead = false;
        
        // Check if interaction contains any dead feature (as negated literal)
        for (const auto& [featureName, selected] : Interaction.Literals) {
            size_t FeatureIdx = Analysis.NameToIndex.at(featureName);
            if (Analysis.DeadFeatures.contains(FeatureIdx) && !selected) {
                ContainsDead = true;
                break;
            }
        }
        
        if (!ContainsDead) {
            Filtered.insert(Interaction);
        }
    }
    
    return Filtered;
}

std::set<Interaction> filterAtomicLiteralSets(
    const std::set<Interaction>& Interactions,
    const FeatureModelAnalysis& Analysis) {

    std::set<Interaction> Filtered;

    std::map<LiteralKey, LiteralKey> LiteralToRepresentative;

    for (const auto& [Representative, AtomicSet] : Analysis.AtomicLiteralSets) {
        if (AtomicSet.size() <= 1) {
            continue;
        }

        for (const auto& Lit : AtomicSet) {
            LiteralToRepresentative[Lit] = Representative;
        }
    }

    for (const auto& Interaction : Interactions) {
        bool ContainsNonRepresentative = false;

        for (const auto& [FeatureName, Selected] : Interaction.Literals) {
            size_t FeatureIdx = Analysis.NameToIndex.at(FeatureName);
            LiteralKey Current{.FeatureIdx=FeatureIdx, .Value=Selected};

            auto It = LiteralToRepresentative.find(Current);
            if (It != LiteralToRepresentative.end() && It->second != Current) {
                ContainsNonRepresentative = true;
                break;
            }
        }

        if (!ContainsNonRepresentative) {
            Filtered.insert(Interaction);
        }
    }

    return Filtered;
}

std::set<Interaction> filterParentChildInteractions(
    const std::set<Interaction>& Interactions, 
    const FeatureModelAnalysis& Analysis) {
    
    std::set<Interaction> Filtered;

    for (const auto& Interaction : Interactions) {
        bool ContainsParentChild = false;
        
        // Build set of features in this interaction
        std::set<size_t> FeaturesInInteraction;
        for (const auto& [featureName, selected] : Interaction.Literals) {
            if (selected) {  // Only consider selected features
                FeaturesInInteraction.insert(Analysis.NameToIndex.at(featureName));
            }
        }
        
        // Check if any parent-child pair exists in this interaction
        for (const auto& [parentFeature, childFeature] : Analysis.ParentChildPairs) {
            auto Parent = Analysis.NameToIndex.at(parentFeature->getName().str());
            auto Child = Analysis.NameToIndex.at(childFeature->getName().str());
            if (FeaturesInInteraction.contains(Parent) &&
                FeaturesInInteraction.contains(Child)) {
                ContainsParentChild = true;
                break;
            }
        }
        
        if (!ContainsParentChild) {
            Filtered.insert(Interaction);
        }
    }
 
    return Filtered;
}


// Combined filters

std::set<Interaction> composeFilters(const std::set<Interaction>& Interactions,
                                    const FeatureModelAnalysis& Analysis,
                                    const std::vector<FilterFunction>& Filters) {
    std::set<Interaction> Result = Interactions;
    
    for (const auto& Filter : Filters) {
        Result = Filter(Result, Analysis);
    }
    
    return Result;
}

std::set<Interaction> filterMF_DF(const std::set<Interaction>& Interactions,
                                 const FeatureModelAnalysis& Analysis) {
    auto Temp = filterMandatoryFeatures(Interactions, Analysis);
    return filterDeadFeatures(Temp, Analysis);
}

std::set<Interaction> filterALS_PCI(const std::set<Interaction>& Interactions,
                                            const FeatureModelAnalysis& Analysis) {
    auto Temp = filterAtomicLiteralSets(Interactions, Analysis);
    return filterParentChildInteractions(Temp, Analysis);
}

std::set<Interaction> filterMF_DF_ALS(const std::set<Interaction>& Interactions,
                                     const FeatureModelAnalysis& Analysis) {
    auto Temp = filterMF_DF(Interactions, Analysis);
    return filterAtomicLiteralSets(Temp, Analysis);
}

std::set<Interaction> filterMF_DF_PCI(const std::set<Interaction>& Interactions,
                                 const FeatureModelAnalysis& Analysis) {
    auto Temp = filterMF_DF(Interactions, Analysis);
    return filterParentChildInteractions(Temp, Analysis);
}

std::set<Interaction> filterMF_DF_ALS_PCI(const std::set<Interaction>& Interactions,
                                            const FeatureModelAnalysis& Analysis) {
    auto Temp = filterMF_DF_ALS(Interactions, Analysis);
    return filterParentChildInteractions(Temp, Analysis);
}


// Metric factory

CoverageMetric MetricFactory::createMDAP() {
    // MDAP: MF + DF + ALS + PCI
    return {"MDAP_MF-DF-ALS-PCI", filterMF_DF_ALS_PCI};
}

CoverageMetric MetricFactory::createMDA() {
    // MDA: MF + DF + ALS
    return {"MDA_MF-DF-ALS", filterMF_DF_ALS};
}

CoverageMetric MetricFactory::createMDP() {
    // MDP: MF + DF + PCI
    return {"MDP_MF-DF-PCI", filterMF_DF_PCI};
}

CoverageMetric MetricFactory::createMD() {
    // MD: MF + DF
    return {"MD_MF-DF", filterMF_DF};
}

CoverageMetric MetricFactory::createAP() {
    // AP: ALS + PCI
    return {"AP_ALS-PCI", filterALS_PCI};
}

CoverageMetric MetricFactory::createPCI() {
    // PCI
    return {"PCI", filterParentChildInteractions};
}

CoverageMetric MetricFactory::createALS() {
    // ALS
    return {"ALS", filterAtomicLiteralSets};
}

CoverageMetric MetricFactory::createDefault() {
    // Default (no filters)
    return {"Default", filterDefault};
}


// Helper methods

std::set<Interaction> extractCoveredInteractions(
    const std::vector<Configuration>& Sample, // S
    const FeatureModelAnalysis& Analysis,
    size_t T) {
    
    std::set<Interaction> CoveredInteractions;
    
    // For each complete configuration γ ∈ S
    for (const auto& Config : Sample) {
        // Extract all t-wise partial configs from γ
        auto Interactions = configurationToInteractions(Config, Analysis, T); //⟦γ⟧

        // Add all to covered set
        CoveredInteractions.insert(Interactions.begin(), Interactions.end());
    }
    
    return CoveredInteractions; // {i | ∃γ ∈ S : i ∈ ⟦γ⟧}
}

std::set<Interaction> configurationToInteractions(
    const Configuration& Config,
    const FeatureModelAnalysis& Analysis,
    size_t T) {
    
    std::set<Interaction> Interactions;
    
    // Get all feature indices
    std::vector<size_t> FeatureIndices;
    FeatureIndices.reserve(Config.size());
    for (size_t I = 0; I < Config.size(); ++I) {
        FeatureIndices.push_back(I);
    }
    
    // Generates all t-wise combinations
    std::function<void(size_t, std::vector<size_t>&)> GenerateCombinations;
    GenerateCombinations = [&](size_t Start, std::vector<size_t>& Current) {
        if (Current.size() == T) {
            // Creates interaction from current combination
            Interaction Interaction;
            for (size_t Idx : Current) {
                if (Idx < Config.size() && Analysis.IndexToName.contains(Idx)) {
                    std::string FeatureName = Analysis.IndexToName.at(Idx);
                    Interaction.Literals[FeatureName] = Config[Idx];
                }
            }
            if (Interaction.Literals.size() == T) {
                Interactions.insert(Interaction);
            }
            return;
        }
        
        for (size_t I = Start; I < FeatureIndices.size(); ++I) {
            Current.push_back(FeatureIndices[I]);
            GenerateCombinations(I + 1, Current);
            Current.pop_back();
        }
    };
    
    std::vector<size_t> Current;
    GenerateCombinations(0, Current);
    
    return Interactions;
}

} // namespace coverage

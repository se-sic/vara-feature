#ifndef COVERAGE_METRICS_H
#define COVERAGE_METRICS_H

#include "BDDFeats.h"
#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace coverage {

struct LiteralKey {
    size_t FeatureIdx;
    bool Value;

    auto operator<=>(const LiteralKey&) const = default;
};

// Interaction represents a t-wise feature interaction
struct Interaction {
    // Maps feature name to its state: true (selected), false (deselected)
    std::map<std::string, bool> Literals;
    
    // Provides a strict weak ordering for interactions, required for storage in std::set.
    bool operator<(const Interaction& Other) const {
        return Literals < Other.Literals;
    }
    
    // Compares two interactions for equality.
    bool operator==(const Interaction& Other) const {
        return Literals == Other.Literals;
    }
    
    // Returns the number of literals (feature assignments) in the interaction.
    [[nodiscard]] size_t size() const {
        return Literals.size();
    }
};

// Configuration represents a complete or partial configuration
using Configuration = std::vector<bool>;

// Feature information structure
struct FeatureInfo {
    // Feature name as a string.
    std::string Name;

    // Zero‑based index of the feature.
    size_t Index;
};

using FeaturePair = std::pair<Feature*, Feature*>;

// Feature model analysis results
struct FeatureModelAnalysis {
    // Features: List of all features with their properties.
    std::vector<FeatureInfo> Features;

    // MandatoryFeatures: Set of indices of mandatory features.
    std::set<size_t> MandatoryFeatures;

    // DeadFeatures: Set of indices of dead features.
    std::set<size_t> DeadFeatures;

    // Map from a representative literal (feature index, value) 
    // to the set of all literals that are equivalent under the feature model constraints.
    std::map<LiteralKey, std::set<LiteralKey>> AtomicLiteralSets;

    // Set of (parent, child) feature pairs extracted from the hierarchy.
    std::set<FeaturePair> ParentChildPairs;
    
    // Mapping between feature names and indices
    std::map<std::string, size_t> NameToIndex;
    std::map<size_t, std::string> IndexToName;
};

// Filter function type
using FilterFunction = std::function<std::set<Interaction>(const std::set<Interaction>&, 
                                                           const FeatureModelAnalysis&)>;

// Coverage metric definition
class CoverageMetric {
public:
    CoverageMetric(std::string  Name, FilterFunction Filter)
        : Name(std::move(Name)), Filter(std::move(Filter)) {}
    
    // Compute coverage for a sample
    [[nodiscard]] double compute(const std::set<Interaction>& AllValidInteractions,
                   const std::vector<Configuration>& Sample,
                   const FeatureModelAnalysis& Analysis,
                   size_t T) const;
    
    // Returns the name of the metric.
    [[nodiscard]] std::string getName() const { return Name; }

private:
    std::string Name;
    FilterFunction Filter;
};


// Filter implementations

// 1. Default filter - returns the input set unchanged. (identity function)
std::set<Interaction> filterDefault(const std::set<Interaction>& Interactions,
                                   const FeatureModelAnalysis& Analysis);

// 2. Exclude mandatory features (MF) Removes any interaction that contains a mandatory feature with value true.
std::set<Interaction> filterMandatoryFeatures(const std::set<Interaction>& Interactions,
                                        const FeatureModelAnalysis& Analysis);

// 3. Exclude dead features (DF) Removes any interaction that contains a dead feature with value false.
std::set<Interaction> filterDeadFeatures(const std::set<Interaction>& Interactions,
                                        const FeatureModelAnalysis& Analysis);

// 4. Merge atomic literal sets (ALS) Replaces literals that belong to an atomic set with a representative literal, effectively merging equivalent interactions.
std::set<Interaction> filterAtomicLiteralSets(const std::set<Interaction>& Interactions,
                                             const FeatureModelAnalysis& Analysis);

// 5. Exclude parent-child interactions (PCI) Removes interactions that simultaneously select a parent feature and any of its children.
std::set<Interaction> filterParentChildInteractions(const std::set<Interaction>& Interactions,
                                                   const FeatureModelAnalysis& Analysis);



// Combined filters

// Combine multiple filters using function composition
std::set<Interaction> composeFilters(const std::set<Interaction>& Interactions,
                                    const FeatureModelAnalysis& Analysis,
                                    const std::vector<FilterFunction>& Filters);

// Applies mandatory‑feature filter, then dead‑feature filter.
std::set<Interaction> filterMF_DF(const std::set<Interaction>& Interactions,
                                 const FeatureModelAnalysis& Analysis);

// Applies ALS and PCI filters.
std::set<Interaction> filterALS_PCI(const std::set<Interaction>& Interactions,
                                 const FeatureModelAnalysis& Analysis);                                 

// Applies MF, DF, and ALS filters.
std::set<Interaction> filterMF_DF_ALS(const std::set<Interaction>& Interactions,
                                     const FeatureModelAnalysis& Analysis);

// Applies MF, DF, and PCI filters.
std::set<Interaction> filterMF_DF_PCI(const std::set<Interaction>& Interactions,
                                     const FeatureModelAnalysis& Analysis);

// Applies all four filters (MF, DF, ALS, PCI).
std::set<Interaction> filterMF_DF_ALS_PCI(const std::set<Interaction>& Interactions,
                                            const FeatureModelAnalysis& Analysis);


// Metrics

class MetricFactory {
public:
    // Creates metric MDAP (MF+DF+ALS+PCI).
    static CoverageMetric createMDAP();
    
    // Creates metric MDA (MF+DF+ALS).
    static CoverageMetric createMDA();
    
    // Creates metric MDP (MF+DF+PCI).
    static CoverageMetric createMDP();
    
    // Creates metric MD (MF+DF).
    static CoverageMetric createMD();

    // Creates metric AP (ALS+PCI).
    static CoverageMetric createAP();
    
    // Creates metric PCI.
    static CoverageMetric createPCI();
    
    // Creates metric ALS.
    static CoverageMetric createALS();

    // Creates metric Default (no filters).
    static CoverageMetric createDefault();
};


// Helper methods

// From a sample of configurations, extracts all T-wise interactions that are covered (i.e., that appear in at least one configuration).
std::set<Interaction> extractCoveredInteractions(
    const std::vector<Configuration>& Sample,
    const FeatureModelAnalysis& Analysis,
    size_t T);

// Converts a single full configuration into the set of all T-wise interactions it contains.
std::set<Interaction> configurationToInteractions(
    const Configuration& Config,
    const FeatureModelAnalysis& Analysis,
    size_t T);

} // namespace coverage

#endif // COVERAGE_METRICS_H

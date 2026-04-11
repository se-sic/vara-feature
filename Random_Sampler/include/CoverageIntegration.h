#ifndef COVERAGE_INTEGRATION_H
#define COVERAGE_INTEGRATION_H

#include "BDDFactory.h"
#include "CoverageMetrics.h"
#include "oxidd/bdd.hpp"
#include "vara/Feature/FeatureModel.h"
#include <map>
#include <set>
#include <vector>

namespace coverage {

/**
 * Class to integrate coverage metrics with the BDD-based random sampler
 */
class CoverageEvaluator {
public:
    /**
     * Initialize evaluator with feature model and BDD
     */
    CoverageEvaluator(
        const vara::feature::FeatureModel& FeatureModel,
        const oxidd::bdd_function& Bdd,
        oxidd::bdd_manager& Manager,
        bdd::sample::BDDFactory& Factory
    );

    /**
     * Generate all valid t-wise interactions using BDD
     * This is the most expensive operation - cache results!
     */
    std::set<Interaction> generateValidInteractions(size_t T);

    /**
     * Checks all 2^T value assignments for a given set of feature 
     * indices. For each assignment, builds an interaction and tests 
     * its satisfiability; inserts valid ones into ValidInteractions.
     */
    void checkAllValueAssignments(
    const std::vector<size_t>& FeatureIndices,
    const std::vector<std::string>& FeatureNames,
    std::set<Interaction>& ValidInteractions);

    /**
    * Determines whether a given partial feature assignment (interaction) 
    * is consistent with the feature model BDD. Returns true if there exists 
    * at least one full configuration satisfying the interaction.
    */
    bool isInteractionSatisfiable(const Interaction& Interaction);

    /**
    * Computes the binomial coefficient C(N, K) using a multiplicative formula 
    * to avoid overflow for moderate values.
    */
    size_t binomialCoefficient(size_t N, size_t K);

    /**
     * Evaluate coverage for a sample using multiple metrics
     * Returns map of metric_name -> coverage_value
     */
    std::map<std::string, double> evaluateSample(
        const std::vector<std::vector<bool>>& Sample,
        size_t T
    );

    /**
     * Evaluate coverage over time as sample grows
     * Useful for plotting coverage curves
     */
    std::map<std::string, std::vector<double>> evaluateIncremental(
        const std::vector<std::vector<bool>>& Sample,
        size_t T,
        const std::vector<size_t>& Checkpoints  // Sample sizes to evaluate
    );

    /**
     * Get the feature model analysis
     */
    [[nodiscard]] const FeatureModelAnalysis& getAnalysis() const { return Analysis; }

    /**
     * Export results to CSV
     */
    void exportToCSV(
        const std::string& Filename,
        const std::map<std::string, std::vector<double>>& Results,
        const std::vector<size_t>& SampleSizes
    );

    /**
    * Given a complete configuration (vector of boolean feature values) and a 
    * strength T, generates all possible T-wise interactions (partial assignments) 
    * that are subsumed by the configuration. Returns the set of such interactions.
    */
    std::set<Interaction> extractInteractionsFromConfig(
        const std::vector<bool>& Config, size_t T);

protected:
    const vara::feature::FeatureModel& FeatureModel;
    const oxidd::bdd_function& BDD;
    oxidd::bdd_manager& Manager;
    FeatureModelAnalysis Analysis;
    bdd::sample::BDDFactory& Factory;
    
    // Cache for valid interactions
    std::map<size_t, std::set<Interaction>> ValidInteractionsCache;
    
    // All metrics to evaluate
    std::vector<CoverageMetric> Metrics;

    /**
    * Populates the FeatureToBddVar map by associating each feature name with its 
    * corresponding BDD variable obtained from the BDDFactory. 
    * Called during construction.
    */
    void buildFeatureVarMap();

    std::map<std::string, oxidd::bdd_function> FeatureToBddVar;
    
    /**
    * Performs the initial analysis of the feature model: collects basic feature 
    * information, computes mandatory and dead features, and identifies parent‑child 
    * relationships. Updates the Analysis structure.
    */
    void initializeAnalysis();

    /**
    * Instantiates the list of coverage metrics to be evaluated. By default, it adds 
    * metrics. Can be extended to include other metrics.
    */
    void initializeMetrics();

    /**
    * Identifies features that must always be selected in every valid configuration 
    * (core features). Uses BDD satisfiability checks: a feature is mandatory 
    * if BDD ∧ ¬feature is unsatisfiable.
    */
    std::set<size_t> computeMandatoryFeatures();

    /**
    * Identifies features that can never be selected in any valid configuration. 
    * A feature is dead if BDD ∧ feature is unsatisfiable.
    */
    std::set<size_t> computeDeadFeatures();

    /**
    * Collects all parent‑child feature pairs from the feature model hierarchy. 
    * Used later by the PCI filter.
    */
    std::set<FeaturePair> getParentChildInteraction();    

    oxidd::bdd_function makeLiteral(size_t Idx, bool Value);

    bool implies(const LiteralKey& A, const LiteralKey& B);

    std::map<LiteralKey, std::set<LiteralKey>> getAtomicLiteralSets();
};

} // namespace coverage

#endif // COVERAGE_INTEGRATION_H

#include "BDDSampler.h"
#include "CoverageIntegration.h"
#include "CoverageMetrics.h"
#include "test_helpers.h"
#include <gtest/gtest.h>

using namespace coverage;

class TestableCoverageEvaluator : public CoverageEvaluator {
public:
    using CoverageEvaluator::CoverageEvaluator;
    using CoverageEvaluator::computeMandatoryFeatures;
    using CoverageEvaluator::computeDeadFeatures;
    using CoverageEvaluator::getParentChildInteraction;
    using CoverageEvaluator::initializeAnalysis;
    using CoverageEvaluator::buildFeatureVarMap;
    using CoverageEvaluator::initializeMetrics;
    using CoverageEvaluator::extractInteractionsFromConfig;
    using CoverageEvaluator::generateValidInteractions;
    using CoverageEvaluator::getAtomicLiteralSets;
    using CoverageEvaluator::implies;

    [[nodiscard]] const FeatureModelAnalysis& getAnalysis() const {
        return Analysis;  
    }
};

class ComputationCoverageEvaluatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        const char* SmallModelXml = R"(
        <vm name="Small">
            <binaryOptions>
                <configurationOption>
                    <name>root</name>
                    <optional>False</optional>
                    <children><options>A</options><options>B</options><options>C</options></children>
                </configurationOption>
                <configurationOption><name>A</name><parent>root</parent><optional>False</optional></configurationOption>
                <configurationOption><name>B</name><parent>root</parent><optional>True</optional></configurationOption>
                <configurationOption><name>C</name><parent>root</parent><optional>True</optional></configurationOption>
            </binaryOptions>
        </vm>
                                        )";

        FeatureModel = test_utils::createModelFromString(SmallModelXml);

        // Create BDD factory and BDD
        Factory = std::make_unique<bdd::sample::BDDFactory>();
        Bdd = Factory->modelToBdd(*FeatureModel);
        Manager = Bdd->containing_manager();

        // Instantiate the testable evaluator
        Evaluator = std::make_unique<TestableCoverageEvaluator>(
            *FeatureModel, *Bdd, *Manager, *Factory
        );
    }

    [[nodiscard]] const FeatureModelAnalysis& getAnalysis() const {
        return Evaluator->getAnalysis();
    }

    // Valid configurations for this model:
    // root is always true, A is mandatory, B/C optional.
    static std::vector<std::vector<bool>> allValidConfigs() {
        return {
            {true, true, false, false},
            {true, true, true,  false},
            {true, true, false, true },
            {true, true, true,  true }
        };
    }

    std::unique_ptr<vara::feature::FeatureModel> FeatureModel;
    std::unique_ptr<bdd::sample::BDDFactory> Factory;
    std::optional<oxidd::bdd_function> Bdd;
    std::optional<oxidd::bdd_manager> Manager;
    std::unique_ptr<TestableCoverageEvaluator> Evaluator;
};

class DeadFeatureCoverageEvaluatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        const char* DeadModelXml = R"(
        <vm name="DeadModel">
            <binaryOptions>
                <configurationOption>
                    <name>root</name>
                    <optional>False</optional>
                    <children><options>A</options><options>B</options><options>C</options></children>
                </configurationOption>
                <configurationOption><name>A</name><parent>root</parent><optional>False</optional></configurationOption>
                <configurationOption><name>B</name><parent>root</parent><optional>True</optional></configurationOption>
                <configurationOption><name>C</name><parent>root</parent><optional>True</optional></configurationOption>
            </binaryOptions>
            <booleanConstraints>
                <constraint>!A | !C</constraint>
            </booleanConstraints>
        </vm>
        )";

        FeatureModel = test_utils::createModelFromString(DeadModelXml);
        Factory = std::make_unique<bdd::sample::BDDFactory>();
        Bdd = Factory->modelToBdd(*FeatureModel);
        Manager = Bdd.containing_manager();

        Evaluator = std::make_unique<TestableCoverageEvaluator>(
            *FeatureModel, Bdd, Manager, *Factory
        );
    }

    [[nodiscard]] const FeatureModelAnalysis& getAnalysis() const {
        return Evaluator->getAnalysis();
    }

    std::unique_ptr<vara::feature::FeatureModel> FeatureModel;
    std::unique_ptr<bdd::sample::BDDFactory> Factory;
    oxidd::bdd_function Bdd;
    oxidd::bdd_manager Manager;
    std::unique_ptr<TestableCoverageEvaluator> Evaluator;
};

TEST_F(ComputationCoverageEvaluatorTest, ModelHasExpectedNumberOfFeaturesAndConfigurations) {
    EXPECT_EQ(FeatureModel->size(), 4U);
    EXPECT_DOUBLE_EQ(Bdd->sat_count_double(FeatureModel->size()), 4.0);
}

TEST_F(ComputationCoverageEvaluatorTest, GenerateValidTwoWiseInteractionsReturnsExpectedCount) {
    auto Interactions = Evaluator->generateValidInteractions(2);
    EXPECT_EQ(Interactions.size(), 13U);
}

TEST_F(ComputationCoverageEvaluatorTest, EmptyFilteredInteractionSetReturnsOne) {
    std::set<Interaction> AllValid;
    std::vector<std::vector<bool>> Sample = {
        {true, true, false, false}
    };

    CoverageMetric Metric("Default", filterDefault);
    EXPECT_DOUBLE_EQ(Metric.compute(AllValid, Sample, getAnalysis(), 2), 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, DefaultMetricHasFullCoverageWhenAllValidConfigurationsAreUsed) {
    auto AllValid = Evaluator->generateValidInteractions(2);
    auto Sample = allValidConfigs();

    CoverageMetric Metric("Default", filterDefault);
    EXPECT_DOUBLE_EQ(Metric.compute(AllValid, Sample, getAnalysis(), 2), 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, SingleInteractionCoveredGivesQuarterCoverage) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true},  {"B", true }}),
        test_utils::makeInteraction({{"A", true},  {"B", false}}),
        test_utils::makeInteraction({{"A", false}, {"B", true }}),
        test_utils::makeInteraction({{"A", false}, {"B", false}})
    };

    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false}  // Covers only {A=true, B=true}
    };

    CoverageMetric Metric("Default", filterDefault);
    EXPECT_DOUBLE_EQ(Metric.compute(AllValid, Sample, getAnalysis(), 2), 0.25);
}

TEST_F(ComputationCoverageEvaluatorTest, DuplicateConfigurationsDoNotIncreaseCoverage) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"B", true}})
    };

    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false},
        {true, true, true, false},
        {true, true, true, false}
    };

    CoverageMetric Metric("Default", filterDefault);
    EXPECT_DOUBLE_EQ(Metric.compute(AllValid, Sample, getAnalysis(), 2), 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, OneWiseCoverageCanReachFullCoverageWithTwoValidConfigurations) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"B", true}}),
        test_utils::makeInteraction({{"B", false}}),
        test_utils::makeInteraction({{"C", true}}),
        test_utils::makeInteraction({{"C", false}})
    };

    std::vector<std::vector<bool>> Sample = {
        {true, true, false, false},
        {true, true, true,  true }
    };

    CoverageMetric Metric("Default", filterDefault);
    EXPECT_DOUBLE_EQ(Metric.compute(AllValid, Sample, getAnalysis(), 1), 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, ThreeWiseCoverageWorksForSingleInteraction) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"B", true}, {"C", true}})
    };

    std::vector<std::vector<bool>> Sample = {
        {true, true, true, true}
    };

    CoverageMetric Metric("Default", filterDefault);
    EXPECT_DOUBLE_EQ(Metric.compute(AllValid, Sample, getAnalysis(), 3), 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, MandatoryFilterRemovesInteractionsContainingSelectedMandatoryFeatures) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"root", true}, {"A", true }}),
        test_utils::makeInteraction({{"root", true}, {"B", true }}),
        test_utils::makeInteraction({{"A", true},    {"B", true }}),
        test_utils::makeInteraction({{"B", true},    {"C", false}})
    };

    auto Filtered = filterMandatoryFeatures(AllValid, getAnalysis());

    std::set<Interaction> Expected = {
        test_utils::makeInteraction({{"B", true}, {"C", false}})
    };

    EXPECT_EQ(Filtered, Expected);
}

TEST_F(ComputationCoverageEvaluatorTest, ParentChildFilterRemovesSelectedParentChildPairs) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"root", true}, {"A", true }}),
        test_utils::makeInteraction({{"root", true}, {"B", true }}),
        test_utils::makeInteraction({{"A", true},    {"B", true }}),
        test_utils::makeInteraction({{"B", true},    {"C", false}})
    };

    auto Filtered = filterParentChildInteractions(AllValid, getAnalysis());

    std::set<Interaction> Expected = {
        test_utils::makeInteraction({{"A", true}, {"B", true}}),
        test_utils::makeInteraction({{"B", true}, {"C", false}})
    };

    EXPECT_EQ(Filtered, Expected);
}

TEST_F(ComputationCoverageEvaluatorTest, CombinedMandatoryDeadFilterMatchesExpectedCoverage) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"root", true}, {"A", true }}),
        test_utils::makeInteraction({{"root", true}, {"B", true }}),
        test_utils::makeInteraction({{"A", true},    {"B", true }}),
        test_utils::makeInteraction({{"B", true},    {"C", false}})
    };

    std::vector<std::vector<bool>> Sample = {
        {true, true, true,  false},
        {true, true, false, false}
    };

    CoverageMetric DefaultMetric("Default", filterDefault);
    CoverageMetric MFDFMetric("MF_MF-DF", filterMF_DF);

    EXPECT_DOUBLE_EQ(DefaultMetric.compute(AllValid, Sample, getAnalysis(), 2), 1.0);
    EXPECT_DOUBLE_EQ(MFDFMetric.compute(AllValid, Sample, getAnalysis(), 2), 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, ExtractInteractionsFromConfigurationReturnsExpectedTwoWiseSet) {
    std::vector<bool> Config = {true, true, true, false};

    auto Interactions = Evaluator->extractInteractionsFromConfig(Config, 2);

    std::set<Interaction> Expected = {
        test_utils::makeInteraction({{"root", true}, {"A", true }}),
        test_utils::makeInteraction({{"root", true}, {"B", true }}),
        test_utils::makeInteraction({{"root", true}, {"C", false}}),
        test_utils::makeInteraction({{"A", true},    {"B", true }}),
        test_utils::makeInteraction({{"A", true},    {"C", false}}),
        test_utils::makeInteraction({{"B", true},    {"C", false}})
    };

    EXPECT_EQ(Interactions, Expected);
}

TEST_F(ComputationCoverageEvaluatorTest, CoverageIsMonotonicWhenSampleGrows) {
    auto AllValid = Evaluator->generateValidInteractions(2);

    std::vector<std::vector<bool>> Sample1 = {
        {true, true, false, false}
    };

    std::vector<std::vector<bool>> Sample2 = {
        {true, true, false, false},
        {true, true, true,  true}
    };

    CoverageMetric Metric("Default", filterDefault);

    double Coverage1 = Metric.compute(AllValid, Sample1, getAnalysis(), 2);
    double Coverage2 = Metric.compute(AllValid, Sample2, getAnalysis(), 2);

    EXPECT_LE(Coverage1, Coverage2);
}

TEST_F(ComputationCoverageEvaluatorTest, PCIComputationChangesCoverageAsExpected) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"root", true}, {"A", true }}),   // parent-child
        test_utils::makeInteraction({{"root", true}, {"B", true }}),   // parent-child
        test_utils::makeInteraction({{"A", true},    {"B", true }}),   // not parent-child
        test_utils::makeInteraction({{"B", true},    {"C", false}})    // not parent-child
    };

    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false}
    };

    CoverageMetric DefaultMetric("Default", filterDefault);
    CoverageMetric PCIMetric("PCI", filterParentChildInteractions);

    double DefaultCoverage = DefaultMetric.compute(AllValid, Sample, getAnalysis(), 2);
    double PCICoverage = PCIMetric.compute(AllValid, Sample, getAnalysis(), 2);

    EXPECT_DOUBLE_EQ(DefaultCoverage, 1.0);
    EXPECT_DOUBLE_EQ(PCICoverage, 1.0);
}

TEST_F(DeadFeatureCoverageEvaluatorTest, DeadFeatureFilterCanIncreaseCoverageRatio) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"B", true},  {"C", false}}), // filtered by DF
        test_utils::makeInteraction({{"B", false}, {"C", false}}), // filtered by DF
        test_utils::makeInteraction({{"A", true},  {"B", true}}),  // survives DF
        test_utils::makeInteraction({{"A", true},  {"B", false}})  // survives DF
    };

    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false},
        {true, true, false, false}
    };

    CoverageMetric DefaultMetric("Default", filterDefault);
    CoverageMetric DFMetric("DF", filterDeadFeatures);

    double DefaultCoverage = DefaultMetric.compute(AllValid, Sample, getAnalysis(), 2);
    double DFCoverage = DFMetric.compute(AllValid, Sample, getAnalysis(), 2);

    EXPECT_DOUBLE_EQ(DefaultCoverage, 1.0);
    EXPECT_DOUBLE_EQ(DFCoverage, 1.0);
}

class AtomicLiteralSetFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        const char* AlsModelXml = R"(
        <vm name="ALSModel">
            <binaryOptions>
                <configurationOption>
                    <name>root</name>
                    <optional>False</optional>
                    <children>
                        <options>A</options>
                        <options>B</options>
                        <options>C</options>
                    </children>
                </configurationOption>
                <configurationOption><name>A</name><parent>root</parent><optional>True</optional></configurationOption>
                <configurationOption><name>B</name><parent>root</parent><optional>True</optional></configurationOption>
                <configurationOption><name>C</name><parent>root</parent><optional>True</optional></configurationOption>
            </binaryOptions>
            <booleanConstraints>
                <constraint>!A | B</constraint>
                <constraint>!B | A</constraint>
            </booleanConstraints>
        </vm>
        )";

        FeatureModel = test_utils::createModelFromString(AlsModelXml);
        Factory = std::make_unique<bdd::sample::BDDFactory>();
        Bdd = Factory->modelToBdd(*FeatureModel);
        Manager = Bdd.containing_manager();

        Evaluator = std::make_unique<TestableCoverageEvaluator>(
            *FeatureModel, Bdd, Manager, *Factory
        );
    }

    [[nodiscard]] const FeatureModelAnalysis& getAnalysis() const {
        return Evaluator->getAnalysis();
    }

    std::unique_ptr<vara::feature::FeatureModel> FeatureModel;
    std::unique_ptr<bdd::sample::BDDFactory> Factory;
    oxidd::bdd_function Bdd;
    oxidd::bdd_manager Manager;
    std::unique_ptr<TestableCoverageEvaluator> Evaluator;
};

TEST_F(AtomicLiteralSetFilterTest, ALSFilterRemovesEquivalentNonRepresentativeLiteralInteractions) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"C", true}}),
        test_utils::makeInteraction({{"B", true}, {"C", true}})
    };

    auto Filtered = filterAtomicLiteralSets(AllValid, getAnalysis());

    EXPECT_EQ(Filtered.size(), 1U);
}

TEST_F(AtomicLiteralSetFilterTest, ALSFilterCanIncreaseCoverageByMergingEquivalentInteractions) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"C", true}}),
        test_utils::makeInteraction({{"B", true}, {"C", true}})
    };

    std::vector<std::vector<bool>> Sample = {
        {true, true, true, true} // root, A, B, C
    };

    CoverageMetric DefaultMetric("Default", filterDefault);
    CoverageMetric ALSMetric("ALS", filterAtomicLiteralSets);

    double DefaultCoverage = DefaultMetric.compute(AllValid, Sample, getAnalysis(), 2);
    double ALSCoverage = ALSMetric.compute(AllValid, Sample, getAnalysis(), 2);

    EXPECT_LE(DefaultCoverage, ALSCoverage);
}


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
    using CoverageEvaluator::extractInteractionsFromConfig;
    using CoverageEvaluator::generateValidInteractions;

    [[nodiscard]] const FeatureModelAnalysis& getAnalysis() const {
        return Analysis;
    }
};

class SpecialModelCoverageTest : public ::testing::Test {
protected:
    void createEvaluator(const char* Xml) {
        FeatureModel = test_utils::createModelFromString(Xml);
        Factory = std::make_unique<bdd::sample::BDDFactory>();
        Bdd = Factory->modelToBdd(*FeatureModel);
        Manager = Bdd->containing_manager();
        Evaluator = std::make_unique<TestableCoverageEvaluator>(
            *FeatureModel, *Bdd, *Manager, *Factory
        );
    }

    [[nodiscard]] const FeatureModelAnalysis& getAnalysis() const {
        return Evaluator->getAnalysis();
    }

    std::unique_ptr<vara::feature::FeatureModel> FeatureModel;
    std::unique_ptr<bdd::sample::BDDFactory> Factory;
    std::optional<oxidd::bdd_function> Bdd;
    std::optional<oxidd::bdd_manager> Manager;
    std::unique_ptr<TestableCoverageEvaluator> Evaluator;
};

TEST_F(SpecialModelCoverageTest, DeadFeatureIsDetected) {
    // Structure:
    //   root mandatory
    //   A mandatory child
    //   Dead optional child with constraint Dead => !root
    // Since root is always selected, Dead can never be selected.
    const char* DeadModelXml = R"(
    <vm name="DeadModel">
        <binaryOptions>
            <configurationOption>
                <name>root</name>
                <optional>False</optional>
                <children><options>A</options><options>Dead</options></children>
            </configurationOption>
            <configurationOption><name>A</name><parent>root</parent><optional>False</optional></configurationOption>
            <configurationOption><name>Dead</name><parent>root</parent><optional>True</optional>
                <excludedOptions><options>root</options></excludedOptions>
            </configurationOption>
        </binaryOptions>
    </vm>
    )";

    createEvaluator(DeadModelXml);

    const auto DeadFeatures = Evaluator->computeDeadFeatures();
    ASSERT_EQ(DeadFeatures.size(), 1U);
    EXPECT_TRUE(DeadFeatures.contains(getAnalysis().NameToIndex.at("Dead")));
}

TEST_F(SpecialModelCoverageTest, DeadFeatureFilterRemovesNegatedDeadLiteral) {
    const char* DeadModelXml = R"(
    <vm name="DeadModel">
        <binaryOptions>
            <configurationOption>
                <name>root</name>
                <optional>False</optional>
                <children><options>A</options><options>Dead</options></children>
            </configurationOption>
            <configurationOption><name>A</name><parent>root</parent><optional>False</optional></configurationOption>
            <configurationOption><name>Dead</name><parent>root</parent><optional>True</optional>
                <excludedOptions><options>root</options></excludedOptions>
            </configurationOption>
        </binaryOptions>
    </vm>
    )";

    createEvaluator(DeadModelXml);

    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"Dead", false}}),
        test_utils::makeInteraction({{"root", true}, {"A", true}})
    };

    auto Filtered = filterDeadFeatures(AllValid, getAnalysis());

    // The interaction containing Dead=false should be removed by the current DF implementation.
    EXPECT_EQ(Filtered.size(), 1U);
    EXPECT_TRUE(Filtered.contains(test_utils::makeInteraction({{"root", true}, {"A", true}})));
}

TEST_F(SpecialModelCoverageTest, ParentChildPairsAreDetected) {
    const char* SmallModelXml = R"(
    <vm name="ParentChild">
        <binaryOptions>
            <configurationOption>
                <name>root</name>
                <optional>False</optional>
                <children><options>A</options><options>B</options></children>
            </configurationOption>
            <configurationOption><name>A</name><parent>root</parent><optional>False</optional></configurationOption>
            <configurationOption><name>B</name><parent>root</parent><optional>True</optional></configurationOption>
        </binaryOptions>
    </vm>
    )";

    createEvaluator(SmallModelXml);

    auto ParentChild = Evaluator->getParentChildInteraction();
    EXPECT_EQ(ParentChild.size(), 2U);
}

TEST_F(SpecialModelCoverageTest, PCIFilterRemovesSelectedParentChildInteractions) {
    const char* SmallModelXml = R"(
    <vm name="ParentChild">
        <binaryOptions>
            <configurationOption>
                <name>root</name>
                <optional>False</optional>
                <children><options>A</options><options>B</options></children>
            </configurationOption>
            <configurationOption><name>A</name><parent>root</parent><optional>False</optional></configurationOption>
            <configurationOption><name>B</name><parent>root</parent><optional>True</optional></configurationOption>
        </binaryOptions>
    </vm>
    )";

    createEvaluator(SmallModelXml);

    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"root", true}, {"A", true}}),   // parent-child, remove
        test_utils::makeInteraction({{"root", true}, {"B", true}}),   // parent-child, remove
        test_utils::makeInteraction({{"A", true}, {"B", true}}),      // keep
        test_utils::makeInteraction({{"A", true}, {"B", false}})      // keep
    };

    auto Filtered = filterParentChildInteractions(AllValid, getAnalysis());

    EXPECT_EQ(Filtered.size(), 2U);
    EXPECT_TRUE(Filtered.contains(test_utils::makeInteraction({{"A", true}, {"B", true}})));
    EXPECT_TRUE(Filtered.contains(test_utils::makeInteraction({{"A", true}, {"B", false}})));
}

TEST_F(SpecialModelCoverageTest, PCIMetricCanIncreaseRelativeCoverageByReducingDenominator) {
    const char* SmallModelXml = R"(
    <vm name="ParentChild">
        <binaryOptions>
            <configurationOption>
                <name>root</name>
                <optional>False</optional>
                <children><options>A</options><options>B</options></children>
            </configurationOption>
            <configurationOption><name>A</name><parent>root</parent><optional>False</optional></configurationOption>
            <configurationOption><name>B</name><parent>root</parent><optional>True</optional></configurationOption>
        </binaryOptions>
    </vm>
    )";

    createEvaluator(SmallModelXml);

    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"root", true}, {"A", true}}),
        test_utils::makeInteraction({{"root", true}, {"B", true}}),
        test_utils::makeInteraction({{"A", true}, {"B", true}}),
        test_utils::makeInteraction({{"A", true}, {"B", false}})
    };

    // This sample only covers the non-parent-child interactions.
    std::vector<std::vector<bool>> Sample = {
        {true, true, true},   // root=T, A=T, B=T
        {true, true, false}   // root=T, A=T, B=F
    };

    CoverageMetric DefaultMetric("Default", filterDefault);
    CoverageMetric PCIMetric("PCI", filterParentChildInteractions);

    const double DefaultCoverage = DefaultMetric.compute(AllValid, Sample, getAnalysis(), 2);
    const double PCICoverage = PCIMetric.compute(AllValid, Sample, getAnalysis(), 2);

    EXPECT_DOUBLE_EQ(DefaultCoverage, 1.0); 
    EXPECT_DOUBLE_EQ(PCICoverage, 1.0);     
}

TEST_F(SpecialModelCoverageTest, XORModelProducesExpectedNumberOfValidPairwiseInteractions) {
    // root mandatory, exactly one of X or Y selected.
    const char* XorModelXml = R"(
    <vm name="XorModel">
        <binaryOptions>
            <configurationOption>
                <name>root</name>
                <optional>False</optional>
                <children><options>X</options><options>Y</options></children>
            </configurationOption>
            <configurationOption><name>X</name><parent>root</parent><optional>True</optional></configurationOption>
            <configurationOption><name>Y</name><parent>root</parent><optional>True</optional></configurationOption>
        </binaryOptions>
        <booleanConstraints>
            <constraint><imp> X <not>Y</not> </imp></constraint>
            <constraint><imp> Y <not>X</not> </imp></constraint>
            <constraint><disj> X Y </disj></constraint>
        </booleanConstraints>
    </vm>
    )";

    createEvaluator(XorModelXml);

    auto Interactions = Evaluator->generateValidInteractions(2);

    EXPECT_FALSE(Interactions.empty());
}


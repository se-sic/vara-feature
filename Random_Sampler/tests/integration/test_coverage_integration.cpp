#include "CoverageIntegration.h"
#include "BDDFactory.h"
#include "test_helpers.h"
#include <gtest/gtest.h>

using namespace coverage;

class TestableCoverageEvaluator : public CoverageEvaluator {
public:
    using CoverageEvaluator::CoverageEvaluator;
    using CoverageEvaluator::computeMandatoryFeatures;
    using CoverageEvaluator::computeDeadFeatures;
    using CoverageEvaluator::getParentChildInteraction;
    using CoverageEvaluator::buildFeatureVarMap;
    using CoverageEvaluator::initializeMetrics;
};

class CoverageEvaluatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        const char* SmallModelXml = R"(
                                        <vm name="Small">
                                            <binaryOptions>
                                                <configurationOption>
                                                    <name>root</name>
                                                    <optional>False</optional>
                                                    <children> 
                                                        <options>A</options>
                                                        <options>B</options>
                                                    </children>
                                                </configurationOption>
                                                <configurationOption>
                                                    <name>A</name>
                                                    <parent>root</parent>
                                                    <optional>False</optional>
                                                </configurationOption>
                                                <configurationOption>
                                                    <name>B</name>
                                                    <parent>root</parent>
                                                    <optional>True</optional>
                                                </configurationOption>
                                                <configurationOption>
                                                    <name>C</name>
                                                    <parent>root</parent>
                                                    <optional>True</optional>
                                                </configurationOption>
                                            </binaryOptions>
                                            <booleanConstraints>
                                                <constraint>!A | !C</constraint>
                                            </booleanConstraints>
                                        </vm>
                                        )";

        FeatureModel = test_utils::createModelFromString(SmallModelXml);

        // Create BDD factory and BDD
        Factory = std::make_unique<bdd::sample::BDDFactory>();
        Bdd = Factory->modelToBdd(*FeatureModel);
        Manager = Bdd.containing_manager();

        // Instantiate the testable evaluator
        Evaluator = std::make_unique<TestableCoverageEvaluator>(
            *FeatureModel, Bdd, Manager, *Factory
        );
    }

    std::unique_ptr<vara::feature::FeatureModel> FeatureModel;
    std::unique_ptr<bdd::sample::BDDFactory> Factory;
    oxidd::bdd_function Bdd;
    oxidd::bdd_manager Manager;
    std::unique_ptr<TestableCoverageEvaluator> Evaluator;
};

TEST_F(CoverageEvaluatorTest, ComputeMandatoryFeatures) {
    auto Mandatory = Evaluator->computeMandatoryFeatures();

    const auto& Analysis = Evaluator->getAnalysis();
    size_t IndexRoot = Analysis.NameToIndex.at("root");
    size_t IndexA = Analysis.NameToIndex.at("A");
    size_t IndexB = Analysis.NameToIndex.at("B");

    EXPECT_EQ(Mandatory.size(), 2);
    EXPECT_TRUE(Mandatory.contains(IndexRoot));
    EXPECT_TRUE(Mandatory.contains(IndexA));
    EXPECT_FALSE(Mandatory.contains(IndexB));    
}

TEST_F(CoverageEvaluatorTest, ComputeDeadFeatures) {
    auto Dead = Evaluator->computeDeadFeatures();

    const auto& Analysis = Evaluator->getAnalysis();
    size_t IndexRoot = Analysis.NameToIndex.at("root");
    size_t IndexA = Analysis.NameToIndex.at("A");
    size_t IndexB = Analysis.NameToIndex.at("B");
    size_t IndexC = Analysis.NameToIndex.at("C");

    EXPECT_EQ(Dead.size(), 1);
    EXPECT_FALSE(Dead.contains(IndexRoot));
    EXPECT_FALSE(Dead.contains(IndexA));
    EXPECT_FALSE(Dead.contains(IndexB));
    EXPECT_TRUE(Dead.contains(IndexC));
}

TEST_F(CoverageEvaluatorTest, GetParentChildInteraction) {
    auto ParentChildInteractions = Evaluator->getParentChildInteraction();

    std::set<std::pair<std::string, std::string>> ReceivedFeatures;
    for (const auto& Pair : ParentChildInteractions) {
        ReceivedFeatures.insert({
            Pair.first->getName().str(),
            Pair.second->getName().str()
        });
    }

    std::set<std::pair<std::string, std::string>> ExpectedFeatures = {
        {"A", "root"},
        {"B", "root"},
        {"C", "root"}
    };

    EXPECT_EQ(ParentChildInteractions.size(), 3);
    EXPECT_EQ(ExpectedFeatures, ReceivedFeatures);
}
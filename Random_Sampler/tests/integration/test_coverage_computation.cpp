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
        Manager = Bdd.containing_manager();

        // Instantiate the testable evaluator
        Evaluator = std::make_unique<TestableCoverageEvaluator>(
            *FeatureModel, Bdd, Manager, *Factory
        );

        Evaluator->initializeAnalysis();
        Evaluator->buildFeatureVarMap();
        Evaluator->initializeMetrics();
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


TEST_F(ComputationCoverageEvaluatorTest, EmptyInteractions) {
    std::set<Interaction> AllValid;  
    std::vector<std::vector<bool>> Sample = {
        {true, false, true, false}
    };
    
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 2);
    
    // Assert: Empty valid interactions → 100% coverage (edge case handling)
    EXPECT_DOUBLE_EQ(Coverage, 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, EmptySample) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"B", false}}),
        test_utils::makeInteraction({{"A", false}, {"B", true}})
    };
    std::vector<std::vector<bool>> Sample;  
    
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 2);
    
    // Assert: No configs → 0% coverage
    EXPECT_DOUBLE_EQ(Coverage, 0.0);
}

TEST_F(ComputationCoverageEvaluatorTest, FullCoverage) {
    std::set<Interaction> AllValid = Evaluator->generateValidInteractions(2);
    
    std::vector<std::vector<bool>> Sample;
    for (int I = 0; I < 10; ++I) {
        auto Config = bdd::sample::generateConfiguration(
            Manager, Bdd, *Factory, &Factory->SatMap);
        Sample.push_back(Config);
    }
        
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 2);

    
    EXPECT_GT(Coverage, 0.8);
}

TEST_F(ComputationCoverageEvaluatorTest, PartialCoverage) {
    std::set<Interaction> AllValid = Evaluator->generateValidInteractions(2);
    
    // Sample that covers only 2 interactions
    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false},   // Covers {A=T, B=T}
        {true, false, false, false}  // Covers {A=F, B=F}
    };
    
    
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 2);
    
    EXPECT_GT(Coverage, 0.4);  // At least 40%
    EXPECT_LT(Coverage, 0.8);  // But less than 80%
}

TEST_F(ComputationCoverageEvaluatorTest, SingleInteractionCovered) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"B", true}}),
        test_utils::makeInteraction({{"A", true}, {"B", false}}),
        test_utils::makeInteraction({{"A", false}, {"B", true}}),
        test_utils::makeInteraction({{"A", false}, {"B", false}})
    };
    
    // Sample covers only 1 interaction
    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false}  // Only covers {A=T, B=T}
    };
    
    
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 2);
    
    // Assert: 1 out of 4 covered → 25%
    EXPECT_DOUBLE_EQ(Coverage, 0.25);
}

TEST_F(ComputationCoverageEvaluatorTest, FilterExcludesInteractions) {
    std::set<Interaction> AllValid = Evaluator->generateValidInteractions(2);
    
    // Sample covers 4 interactions (not the parent-child ones)
    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false},   // {A=T, B=T}
        {true, true, false, false},  // {A=T, B=F}
        {true, false, true, false},  // {A=F, B=T}
        {true, false, false, false}  // {A=F, B=F}
    };
    
    
    CoverageMetric DefaultMetric("M7_Default", filterDefault);
    
    CoverageMetric PCIMetric("M5_PCI", filterParentChildInteractions);
    
    double CoverageDefault = DefaultMetric.compute(AllValid, Sample, getAnalysis(), 2);
    double CoveragePCI = PCIMetric.compute(AllValid, Sample, getAnalysis(), 2);
    
    EXPECT_GE(CoveragePCI, CoverageDefault * 0.95);  // Allow small rounding
    
    // Both should be reasonable
    EXPECT_GT(CoverageDefault, 0.5);
    EXPECT_GT(CoveragePCI, 0.5);
}

TEST_F(ComputationCoverageEvaluatorTest, MandatoryDeadFilter) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"root", true}, {"A", true}}),   // Contains mandatory root
        test_utils::makeInteraction({{"root", true}, {"A", false}}),  // Contains mandatory root
        test_utils::makeInteraction({{"A", true}, {"B", true}}),      // No mandatory/dead
        test_utils::makeInteraction({{"A", false}, {"B", false}})     // No mandatory/dead
    };
    
    // Sample that covers all 4 interactions
    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false},   
        {true, false, false, false}
    };
        
    CoverageMetric DefaultMetric("M7_Default", filterDefault);
    
    CoverageMetric MFDFMetric("M4_MF-DF", filterMandatoryFeatures);
    
    double CoverageDefault = DefaultMetric.compute(AllValid, Sample, getAnalysis(), 2);
    double CoverageMFDF = MFDFMetric.compute(AllValid, Sample, getAnalysis(), 2);
    
    EXPECT_DOUBLE_EQ(CoverageDefault, 4.0 / 4.0);  // 100% - all covered
    EXPECT_DOUBLE_EQ(CoverageMFDF, 2.0 / 2.0);     // 100% - but only 2 after filter
}

TEST_F(ComputationCoverageEvaluatorTest, SingleInteractionInSet) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"B", true}})
    };
    
    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false} 
    };
        
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 2);
    
    EXPECT_DOUBLE_EQ(Coverage, 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, SingleInteractionNotCovered) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"B", true}})
    };
    
    std::vector<std::vector<bool>> Sample = {
        {true, false, false, false}  // A=F, B=F - doesn't cover it
    };
        
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 2);
    
    EXPECT_DOUBLE_EQ(Coverage, 0.0);
}

TEST_F(ComputationCoverageEvaluatorTest, DuplicateConfigsInSample) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"B", true}})
    };
    
    // Same config repeated 3 times
    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false},
        {true, true, true, false},
        {true, true, true, false}
    };
        
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 2);
    
    // Should still be 100% 
    EXPECT_DOUBLE_EQ(Coverage, 1.0);
}


TEST_F(ComputationCoverageEvaluatorTest, OnewiseCoverage) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}}),
        test_utils::makeInteraction({{"A", false}}),
        test_utils::makeInteraction({{"B", true}}),
        test_utils::makeInteraction({{"B", false}})
    };
    
    // Sample covers 3 out of 4
    std::vector<std::vector<bool>> Sample = {
        {true, true, false, false},   // A=T, B=F
        {true, false, true, false}    // A=F, B=T
        // Missing: {A=T, B=T} and {A=F, B=F}
    };
        
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 1);
    
    // Covers: A=T, A=F, B=T, B=F → all 4
    EXPECT_DOUBLE_EQ(Coverage, 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, ThreewiseCoverage) {
    std::set<Interaction> AllValid = {
        test_utils::makeInteraction({{"A", true}, {"B", true}, {"C", true}})
    };
    
    std::vector<std::vector<bool>> Sample = {
        {true, true, true, true}  // Covers A=T, B=T, C=T
    };
        
    CoverageMetric Metric("M7_Default", filterDefault);
    
    double Coverage = Metric.compute(AllValid, Sample, getAnalysis(), 3);
    
    EXPECT_DOUBLE_EQ(Coverage, 1.0);
}

TEST_F(ComputationCoverageEvaluatorTest, DEBUG_ExtractCovered) {
    std::vector<std::vector<bool>> Sample = {
        {true, true, true, false}
    };
    
    // Manually extract
    std::set<Interaction> Covered;
    for (const auto& Config : Sample) {
        auto Extracted = Evaluator->extractInteractionsFromConfig(Config, 2);
        Covered.insert(Extracted.begin(), Extracted.end());
    }
    
    std::cout << "\\n=== Covered Set Size: " << Covered.size() << " ===\\n";
    for (const auto& Interaction : Covered) {
        std::cout << "  {";
        for (const auto& [feature, value] : Interaction.Literals) {
            std::cout << feature << "=" << (value ? "T" : "F") << " ";
        }
        std::cout << "}\\n";
    }
    
    // Now test if we can find them in allValid
    auto AllValid = Evaluator->generateValidInteractions(2);
    
    std::cout << "\\n=== Checking Matches ===\\n";
    for (const auto& CoveredInt : Covered) {
        bool Found = AllValid.contains(CoveredInt);
        std::cout << "  ";
        for (const auto& [feature, value] : CoveredInt.Literals) {
            std::cout << feature << "=" << (value ? "T" : "F") << " ";
        }
        std::cout << " → " << (Found ? "FOUND ✓" : "NOT FOUND ✗") << "\\n";
    }
}
#include "TWiseSampling.h"
#include "BDDFactory.h"
#include "CoverageMetrics.h"
#include "test_helpers.h"

#include <gtest/gtest.h>
#include <optional>
#include <set>
#include <vector>

using twise::Configuration;

class TWiseSamplingTest : public ::testing::Test {
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
                        <options>C</options>
                    </children>
                </configurationOption>
                <configurationOption><name>A</name><parent>root</parent><optional>False</optional></configurationOption>
                <configurationOption><name>B</name><parent>root</parent><optional>True</optional></configurationOption>
                <configurationOption><name>C</name><parent>root</parent><optional>True</optional></configurationOption>
            </binaryOptions>
        </vm>
        )";

        FeatureModel = test_utils::createModelFromString(SmallModelXml);
        Factory = std::make_unique<bdd::sample::BDDFactory>();
        Bdd = Factory->modelToBdd(*FeatureModel);
        Manager = Bdd->containing_manager();
    }

    static std::vector<Configuration> allValidConfigs() {
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
};

class TWiseSamplingImplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        const char* ModelXml = R"(
        <vm name="ImplicationModel">
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
            </booleanConstraints>
        </vm>
        )";

        FeatureModel = test_utils::createModelFromString(ModelXml);
        Factory = std::make_unique<bdd::sample::BDDFactory>();
        Bdd = Factory->modelToBdd(*FeatureModel);
        Manager = Bdd->containing_manager();
    }

    std::unique_ptr<vara::feature::FeatureModel> FeatureModel;
    std::unique_ptr<bdd::sample::BDDFactory> Factory;
    std::optional<oxidd::bdd_function> Bdd;
    std::optional<oxidd::bdd_manager> Manager;
};

TEST_F(TWiseSamplingTest, EnumerateAllConfigurationsReturnsExpectedConfigurations) {
    auto Configs = twise::enumerateAllConfigurations(*Manager, *Bdd);

    auto Expected = allValidConfigs();

    EXPECT_EQ(Configs.size(), Expected.size());

    std::set<Configuration> ActualSet(Configs.begin(), Configs.end());
    std::set<Configuration> ExpectedSet(Expected.begin(), Expected.end());

    EXPECT_EQ(ActualSet, ExpectedSet);
}

TEST_F(TWiseSamplingTest, EnumeratedConfigurationCountMatchesSatCount) {
    auto Configs = twise::enumerateAllConfigurations(*Manager, *Bdd);

    EXPECT_EQ(Configs.size(), 4U);
    EXPECT_DOUBLE_EQ(Bdd->sat_count_double(FeatureModel->size()), 4.0);
}

TEST_F(TWiseSamplingTest, GenerateValidTwoWiseInteractionsReturnsExpectedCount) {
    auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, *Manager);
    auto Interactions = twise::generateValidInteractions(
        2, *FeatureModel, *Bdd, FeatureToBddVar
    );

    EXPECT_EQ(Interactions.size(), 13U);
}

TEST_F(TWiseSamplingTest, ConfigurationCoverCheckWorks) {
    auto Maps = twise::buildFeatureMaps(*FeatureModel);

    coverage::Interaction Interaction;
    Interaction.Literals["A"] = true;
    Interaction.Literals["B"] = false;

    Configuration Config1 = {true, true, false, false};
    Configuration Config2 = {true, true, true, false};

    EXPECT_TRUE(twise::configurationCoversInteraction(Config1, Interaction, Maps));
    EXPECT_FALSE(twise::configurationCoversInteraction(Config2, Interaction, Maps));
}

TEST_F(TWiseSamplingTest, GreedySamplerSelectsOnlyEnumeratedConfigurations) {
    auto Maps = twise::buildFeatureMaps(*FeatureModel);
    auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, *Manager);

    auto AllConfigs = twise::enumerateAllConfigurations(*Manager, *Bdd);
    auto Candidates = twise::generateValidInteractions(2, *FeatureModel, *Bdd, FeatureToBddVar);
    std::vector<coverage::Interaction> CandidateList(Candidates.begin(), Candidates.end());

    auto CoveredIds = twise::precomputeCoveredIdsPerConfig(AllConfigs, CandidateList, Maps);
    auto SelectedIdxs = twise::greedyTWiseSamplingWithIds(
        AllConfigs, CoveredIds, CandidateList.size()
    );

    std::set<Configuration> AllConfigSet(AllConfigs.begin(), AllConfigs.end());

    for (size_t Idx : SelectedIdxs) {
        EXPECT_LT(Idx, AllConfigs.size());
        EXPECT_TRUE(AllConfigSet.contains(AllConfigs[Idx]));
    }
}

TEST_F(TWiseSamplingTest, GreedyTwoWiseSampleCoversAllTwoWiseCandidates) {
    auto Maps = twise::buildFeatureMaps(*FeatureModel);
    auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, *Manager);

    auto AllConfigs = twise::enumerateAllConfigurations(*Manager, *Bdd);
    auto Candidates = twise::generateValidInteractions(2, *FeatureModel, *Bdd, FeatureToBddVar);
    std::vector<coverage::Interaction> CandidateList(Candidates.begin(), Candidates.end());

    auto CoveredIds = twise::precomputeCoveredIdsPerConfig(AllConfigs, CandidateList, Maps);
    auto SelectedIdxs = twise::greedyTWiseSamplingWithIds(
        AllConfigs, CoveredIds, CandidateList.size()
    );

    std::vector<bool> Covered(CandidateList.size(), false);

    for (size_t ConfigIdx : SelectedIdxs) {
        for (size_t Id : CoveredIds[ConfigIdx]) {
            Covered[Id] = true;
        }
    }

    for (bool IsCovered : Covered) {
        EXPECT_TRUE(IsCovered);
    }
}

TEST_F(TWiseSamplingTest, GreedyOneWiseSampleHasExpectedSize) {
    auto Maps = twise::buildFeatureMaps(*FeatureModel);
    auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, *Manager);

    auto AllConfigs = twise::enumerateAllConfigurations(*Manager, *Bdd);
    auto Candidates = twise::generateValidInteractions(1, *FeatureModel, *Bdd, FeatureToBddVar);
    std::vector<coverage::Interaction> CandidateList(Candidates.begin(), Candidates.end());

    auto CoveredIds = twise::precomputeCoveredIdsPerConfig(AllConfigs, CandidateList, Maps);
    auto SelectedIdxs = twise::greedyTWiseSamplingWithIds(
        AllConfigs, CoveredIds, CandidateList.size()
    );

    EXPECT_EQ(SelectedIdxs.size(), 2U);
}

TEST_F(TWiseSamplingTest, GreedyTwoWiseSampleHasReasonableSize) {
    auto Maps = twise::buildFeatureMaps(*FeatureModel);
    auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, *Manager);

    auto AllConfigs = twise::enumerateAllConfigurations(*Manager, *Bdd);
    auto Candidates = twise::generateValidInteractions(2, *FeatureModel, *Bdd, FeatureToBddVar);
    std::vector<coverage::Interaction> CandidateList(Candidates.begin(), Candidates.end());

    auto CoveredIds = twise::precomputeCoveredIdsPerConfig(AllConfigs, CandidateList, Maps);
    auto SelectedIdxs = twise::greedyTWiseSamplingWithIds(
        AllConfigs, CoveredIds, CandidateList.size()
    );

    EXPECT_GT(SelectedIdxs.size(), 0U);
    EXPECT_LE(SelectedIdxs.size(), AllConfigs.size());
}

TEST_F(TWiseSamplingImplicationTest, ImpossibleInteractionIsExcludedFromValidTwoWiseSet) {
    auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, *Manager);
    auto Interactions = twise::generateValidInteractions(
        2, *FeatureModel, *Bdd, FeatureToBddVar
    );

    coverage::Interaction Impossible;
    Impossible.Literals["A"] = true;
    Impossible.Literals["B"] = false;

    EXPECT_FALSE(Interactions.contains(Impossible));
}

TEST_F(TWiseSamplingImplicationTest, ValidInteractionsRemainPresentUnderImplication) {
    auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, *Manager);
    auto Interactions = twise::generateValidInteractions(
        2, *FeatureModel, *Bdd, FeatureToBddVar
    );

    coverage::Interaction Valid1;
    Valid1.Literals["A"] = false;
    Valid1.Literals["B"] = false;

    coverage::Interaction Valid2;
    Valid2.Literals["A"] = false;
    Valid2.Literals["B"] = true;

    coverage::Interaction Valid3;
    Valid3.Literals["A"] = true;
    Valid3.Literals["B"] = true;

    EXPECT_TRUE(Interactions.contains(Valid1));
    EXPECT_TRUE(Interactions.contains(Valid2));
    EXPECT_TRUE(Interactions.contains(Valid3));
}

TEST_F(TWiseSamplingTest, GreedyThreeWiseSampleCoversAllThreeWiseCandidates) {
    auto Maps = twise::buildFeatureMaps(*FeatureModel);
    auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, *Manager);

    auto AllConfigs = twise::enumerateAllConfigurations(*Manager, *Bdd);
    auto Candidates = twise::generateValidInteractions(3, *FeatureModel, *Bdd, FeatureToBddVar);
    std::vector<coverage::Interaction> CandidateList(Candidates.begin(), Candidates.end());

    auto CoveredIds = twise::precomputeCoveredIdsPerConfig(AllConfigs, CandidateList, Maps);
    auto SelectedIdxs = twise::greedyTWiseSamplingWithIds(
        AllConfigs, CoveredIds, CandidateList.size()
    );

    std::vector<bool> Covered(CandidateList.size(), false);

    for (size_t ConfigIdx : SelectedIdxs) {
        for (size_t Id : CoveredIds[ConfigIdx]) {
            Covered[Id] = true;
        }
    }

    for (bool IsCovered : Covered) {
        EXPECT_TRUE(IsCovered);
    }
}

TEST_F(TWiseSamplingTest, GreedyThreeWiseSampleHasReasonableSize) {
    auto Maps = twise::buildFeatureMaps(*FeatureModel);
    auto FeatureToBddVar = twise::buildFeatureVarMap(*FeatureModel, *Manager);

    auto AllConfigs = twise::enumerateAllConfigurations(*Manager, *Bdd);
    auto Candidates = twise::generateValidInteractions(3, *FeatureModel, *Bdd, FeatureToBddVar);
    std::vector<coverage::Interaction> CandidateList(Candidates.begin(), Candidates.end());

    auto CoveredIds = twise::precomputeCoveredIdsPerConfig(AllConfigs, CandidateList, Maps);
    auto SelectedIdxs = twise::greedyTWiseSamplingWithIds(
        AllConfigs, CoveredIds, CandidateList.size()
    );

    EXPECT_GT(SelectedIdxs.size(), 0U);
    EXPECT_LE(SelectedIdxs.size(), AllConfigs.size());
}
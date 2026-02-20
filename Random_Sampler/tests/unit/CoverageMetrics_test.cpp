#include "CoverageMetrics.h"
#include <gtest/gtest.h>

using namespace coverage;


TEST(FilterFeaturesTest, RemovesMandatoryFeatures) {
    FeatureModelAnalysis Analysis;

    std::set<Interaction> Interactions;

    Interaction Interaction1; Interaction1.Literals = {{"A", true}, {"D", true}};
    Interaction Interaction2; Interaction2.Literals = {{"B", true}, {"C", false}};
    Interaction Interaction3; Interaction3.Literals = {{"B", false}, {"C", true}};
    Interaction Interaction4; Interaction4.Literals = {{"A", true}, {"C", false}};

    Interactions = {Interaction1, Interaction2, Interaction3, Interaction4};

    FeatureInfo Feature1 = {.Name="A", .Index=1};
    FeatureInfo Feature2 = {.Name="B", .Index=2};
    FeatureInfo Feature3 = {.Name="C", .Index=3};
    FeatureInfo Feature4 = {.Name="D", .Index=4};

    Analysis.Features.push_back(Feature1);
    Analysis.Features.push_back(Feature2);
    Analysis.Features.push_back(Feature3);
    Analysis.Features.push_back(Feature4);

    Analysis.NameToIndex["A"] = 1;
    Analysis.NameToIndex["B"] = 2;
    Analysis.NameToIndex["C"] = 3;
    Analysis.NameToIndex["D"] = 4;

    Analysis.MandatoryFeatures.insert(1);
    Analysis.MandatoryFeatures.insert(4);

    auto FilteredInteractions = filterMandatoryFeatures(Interactions, Analysis);
    EXPECT_EQ(2, FilteredInteractions.size());
    EXPECT_EQ(FilteredInteractions.count(Interaction1), 0);
    EXPECT_EQ(FilteredInteractions.count(Interaction2), 1);
    EXPECT_EQ(FilteredInteractions.count(Interaction3), 1);
    EXPECT_EQ(FilteredInteractions.count(Interaction4), 0);
}

TEST(FilterFeaturesTest, RemovesDeadFeatures) {
    FeatureModelAnalysis Analysis;

    std::set<Interaction> Interactions;

    Interaction Interaction1; Interaction1.Literals = {{"A", false}, {"D", false}};
    Interaction Interaction2; Interaction2.Literals = {{"B", true}, {"C", false}};
    Interaction Interaction3; Interaction3.Literals = {{"B", false}, {"C", true}};
    Interaction Interaction4; Interaction4.Literals = {{"A", false}, {"C", false}};

    Interactions = {Interaction1, Interaction2, Interaction3, Interaction4};

    FeatureInfo Feature1 = {.Name="A", .Index=1};
    FeatureInfo Feature2 = {.Name="B", .Index=2};
    FeatureInfo Feature3 = {.Name="C", .Index=3};
    FeatureInfo Feature4 = {.Name="D", .Index=4};

    Analysis.Features.push_back(Feature1);
    Analysis.Features.push_back(Feature2);
    Analysis.Features.push_back(Feature3);
    Analysis.Features.push_back(Feature4);

    Analysis.NameToIndex["A"] = 1;
    Analysis.NameToIndex["B"] = 2;
    Analysis.NameToIndex["C"] = 3;
    Analysis.NameToIndex["D"] = 4;

    Analysis.DeadFeatures.insert(1);
    Analysis.DeadFeatures.insert(4);

    auto FilteredInteractions = filterDeadFeatures(Interactions, Analysis);
    EXPECT_EQ(2, FilteredInteractions.size());
    EXPECT_EQ(FilteredInteractions.count(Interaction1), 0);
    EXPECT_EQ(FilteredInteractions.count(Interaction2), 1);
    EXPECT_EQ(FilteredInteractions.count(Interaction3), 1);
    EXPECT_EQ(FilteredInteractions.count(Interaction4), 0);
}


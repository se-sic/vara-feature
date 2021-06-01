#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelBuilder.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Feature, equal) {
  BinaryFeature A0("A");
  BinaryFeature A1("A");
  BinaryFeature A2("a");
  BinaryFeature B("B");

  EXPECT_EQ(A0, A1);
  EXPECT_EQ(A0, A2);
  EXPECT_NE(A0, B);
}

TEST(Feature, locationAccessors) {
  FeatureSourceRange::FeatureSourceLocation Start(3, 4);
  FeatureSourceRange::FeatureSourceLocation End(3, 20);
  BinaryFeature TestLCO("TEST", false,
                        {FeatureSourceRange("path", Start, End)});

  FeatureSourceRange Fsr = *TestLCO.getLocationsBegin();
  EXPECT_EQ(Fsr.getPath(), "path");
  EXPECT_EQ(Fsr.getCategory(), FeatureSourceRange::Category::necessary);
  EXPECT_EQ(Fsr.getStart()->getLineNumber(), 3);
  EXPECT_EQ(Fsr.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(Fsr.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(Fsr.getEnd()->getColumnOffset(), 20);
}

TEST(Feature, locationInit) {
  FeatureSourceRange::FeatureSourceLocation Start(3, 4);
  FeatureSourceRange::FeatureSourceLocation End(3, 20);
  BinaryFeature TestLCO("TEST", false,
                        {FeatureSourceRange("path", Start, End)});

  TestLCO.getLocationsBegin()->getStart()->setLineNumber(4);
  TestLCO.getLocationsBegin()->getStart()->setColumnOffset(2);
  TestLCO.getLocationsBegin()->getEnd()->setLineNumber(4);
  TestLCO.getLocationsBegin()->getEnd()->setColumnOffset(18);

  EXPECT_EQ(TestLCO.getLocationsBegin()->getStart()->getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getLocationsBegin()->getStart()->getColumnOffset(), 2);
  EXPECT_EQ(TestLCO.getLocationsBegin()->getEnd()->getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getLocationsBegin()->getEnd()->getColumnOffset(), 18);
}

TEST(Feature, locationAdd) {
  BinaryFeature TestLCO("TEST", false);
  EXPECT_FALSE(TestLCO.hasLocations());

  FeatureSourceRange::FeatureSourceLocation Start(4, 4);
  FeatureSourceRange::FeatureSourceLocation End(5, 20);
  FeatureSourceRange Location("path2", Start, End);
  TestLCO.addLocation(Location);

  EXPECT_TRUE(TestLCO.hasLocations());
  EXPECT_EQ(TestLCO.getLocationsBegin()->getPath(), "path2");
  EXPECT_EQ(TestLCO.getLocationsBegin()->getStart()->getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getLocationsBegin()->getStart()->getColumnOffset(), 4);
  EXPECT_EQ(TestLCO.getLocationsBegin()->getEnd()->getLineNumber(), 5);
  EXPECT_EQ(TestLCO.getLocationsBegin()->getEnd()->getColumnOffset(), 20);
}

TEST(Feature, locationRemove) {
  FeatureSourceRange::FeatureSourceLocation Start1(3, 4);
  FeatureSourceRange::FeatureSourceLocation End1(3, 20);
  FeatureSourceRange::FeatureSourceLocation Start2(4, 4);
  FeatureSourceRange::FeatureSourceLocation End2(4, 20);
  BinaryFeature TestLCO("TEST", false,
                        {FeatureSourceRange("path1", Start1, End1),
                         FeatureSourceRange("path2", Start2, End2)});

  EXPECT_TRUE(TestLCO.hasLocations());
  FeatureSourceRange Fsr = FeatureSourceRange("path1", Start1, End1);
  TestLCO.removeLocation(Fsr);
  Fsr = FeatureSourceRange("path2", Start2, End2);
  EXPECT_EQ(*TestLCO.getLocationsBegin(), Fsr);
  TestLCO.removeLocation(Fsr);
  EXPECT_FALSE(TestLCO.hasLocations());
}

TEST(Feature, locationUpdate) {
  FeatureSourceRange::FeatureSourceLocation Start1(3, 4);
  FeatureSourceRange::FeatureSourceLocation End1(3, 20);
  FeatureSourceRange::FeatureSourceLocation Start2(4, 4);
  FeatureSourceRange::FeatureSourceLocation End2(4, 20);
  FeatureSourceRange Fsr1 = FeatureSourceRange("path1", Start1, End1);
  FeatureSourceRange Fsr2 = FeatureSourceRange("path2", Start2, End2);
  BinaryFeature TestLCO("TEST", false, {Fsr1});

  EXPECT_TRUE(TestLCO.updateLocation(Fsr1, Fsr2));
  EXPECT_EQ(*TestLCO.getLocationsBegin(), Fsr2);
}

TEST(Feature, getChildren) {
  FeatureModelBuilder B;
  B.makeFeature<NumericFeature>("a", std::vector<int>{1, 2, 3});
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
  B.addEdge("a", "ab")->makeFeature<BinaryFeature>("ab");

  B.emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, "a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(FM->getFeature("a")->getChildren<Feature>(0).size(), 0);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Feature>(1).size(), 0);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Feature>(2).size(), 2);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Feature>(3).size(), 2);
}

} // namespace vara::feature

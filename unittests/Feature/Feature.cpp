#include "vara/Feature/FeatureModel.h"

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
  BinaryFeature TestLCO("TEST", false, FeatureSourceRange("path", Start, End));

  EXPECT_EQ(TestLCO.getLocations()->getStart()->getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getLocations()->getStart()->getColumnOffset(), 4);
  EXPECT_EQ(TestLCO.getLocations()->getEnd()->getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getLocations()->getEnd()->getColumnOffset(), 20);
}

TEST(Feature, locationSetters) {
  FeatureSourceRange::FeatureSourceLocation Start(3, 4);
  FeatureSourceRange::FeatureSourceLocation End(3, 20);
  BinaryFeature TestLCO("TEST", false, FeatureSourceRange("path", Start, End));

  TestLCO.getLocations()->getStart()->setLineNumber(4);
  TestLCO.getLocations()->getStart()->setColumnOffset(2);
  TestLCO.getLocations()->getEnd()->setLineNumber(4);
  TestLCO.getLocations()->getEnd()->setColumnOffset(18);

  EXPECT_EQ(TestLCO.getLocations()->getStart()->getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getLocations()->getStart()->getColumnOffset(), 2);
  EXPECT_EQ(TestLCO.getLocations()->getEnd()->getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getLocations()->getEnd()->getColumnOffset(), 18);
}

TEST(Feature, locationSetter) {
    FeatureSourceRange::FeatureSourceLocation Start(3, 4);
    FeatureSourceRange::FeatureSourceLocation End(3, 20);
    BinaryFeature TestLCO("TEST", false,
                          FeatureSourceRange(Start, End, "path"));

    Start = FeatureSourceRange::FeatureSourceLocation(4, 4);
    End = FeatureSourceRange::FeatureSourceLocation(5,20);
    FeatureSourceRange OtherLocation(Start, End, "path2");
    TestLCO.setFeatureSourceRange(OtherLocation);

    EXPECT_EQ(TestLCO.getLocations()->getPath(), "path2");
    EXPECT_EQ(TestLCO.getLocations()->getStart()->getLineNumber(), 4);
    EXPECT_EQ(TestLCO.getLocations()->getStart()->getColumnOffset(), 4);
    EXPECT_EQ(TestLCO.getLocations()->getEnd()->getLineNumber(), 5);
    EXPECT_EQ(TestLCO.getLocations()->getEnd()->getColumnOffset(), 20);
  }

TEST(Feature, locationSetOnOwnLocation) {
    FeatureSourceRange::FeatureSourceLocation Start(3, 4);
    FeatureSourceRange::FeatureSourceLocation End(3, 20);
    BinaryFeature TestLCO("TEST", false,
                          FeatureSourceRange(Start, End, "path"));

    TestLCO.setFeatureSourceRange(*(TestLCO.getLocations()));
    EXPECT_EQ(TestLCO.getLocations()->getPath(), "path");
    EXPECT_EQ(TestLCO.getLocations()->getStart()->getLineNumber(), 3);
    EXPECT_EQ(TestLCO.getLocations()->getStart()->getColumnOffset(), 4);
    EXPECT_EQ(TestLCO.getLocations()->getEnd()->getLineNumber(), 3);
    EXPECT_EQ(TestLCO.getLocations()->getEnd()->getColumnOffset(), 20);
  }


} // namespace vara::feature

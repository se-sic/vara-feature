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

  EXPECT_EQ(TestLCO.getFeatureSourceRange()->getStart()->getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getFeatureSourceRange()->getStart()->getColumnOffset(), 4);
  EXPECT_EQ(TestLCO.getFeatureSourceRange()->getEnd()->getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getFeatureSourceRange()->getEnd()->getColumnOffset(), 20);
}

TEST(Feature, locationSetters) {
  FeatureSourceRange::FeatureSourceLocation Start(3, 4);
  FeatureSourceRange::FeatureSourceLocation End(3, 20);
  BinaryFeature TestLCO("TEST", false, FeatureSourceRange("path", Start, End));

  TestLCO.getFeatureSourceRange()->getStart()->setLineNumber(4);
  TestLCO.getFeatureSourceRange()->getStart()->setColumnOffset(2);
  TestLCO.getFeatureSourceRange()->getEnd()->setLineNumber(4);
  TestLCO.getFeatureSourceRange()->getEnd()->setColumnOffset(18);

  EXPECT_EQ(TestLCO.getFeatureSourceRange()->getStart()->getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getFeatureSourceRange()->getStart()->getColumnOffset(), 2);
  EXPECT_EQ(TestLCO.getFeatureSourceRange()->getEnd()->getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getFeatureSourceRange()->getEnd()->getColumnOffset(), 18);
}

TEST(Feature, locationSetter) {
    FeatureSourceRange::FeatureSourceLocation Start(3, 4);
    FeatureSourceRange::FeatureSourceLocation End(3, 20);
    BinaryFeature TestLCO("TEST", false, FeatureSourceRange("path", Start, End));

    Start = FeatureSourceRange::FeatureSourceLocation(4, 4);
    End = FeatureSourceRange::FeatureSourceLocation(5,20);
    FeatureSourceRange OtherLocation("path2", Start, End);
    TestLCO.setFeatureSourceRange(OtherLocation);

    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getPath(), "path2");
    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getStart()->getLineNumber(), 4);
    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getStart()->getColumnOffset(), 4);
    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getEnd()->getLineNumber(), 5);
    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getEnd()->getColumnOffset(), 20);
  }

TEST(Feature, locationSetOnOwnLocation) {
    FeatureSourceRange::FeatureSourceLocation Start(3, 4);
    FeatureSourceRange::FeatureSourceLocation End(3, 20);
    BinaryFeature TestLCO("TEST", false, FeatureSourceRange("path", Start, End));

    TestLCO.setFeatureSourceRange(*(TestLCO.getFeatureSourceRange()));
    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getPath(), "path");
    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getStart()->getLineNumber(), 3);
    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getStart()->getColumnOffset(), 4);
    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getEnd()->getLineNumber(), 3);
    EXPECT_EQ(TestLCO.getFeatureSourceRange()->getEnd()->getColumnOffset(), 20);
  }


} // namespace vara::feature

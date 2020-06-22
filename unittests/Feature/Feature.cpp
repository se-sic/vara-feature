#include "vara/Feature/FeatureModel.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Feature, equal) {
  BinaryFeature A0("A");
  BinaryFeature A1("A");
  BinaryFeature A2("a");
  BinaryFeature B("B");

  EXPECT_TRUE(A0 == A1);
  EXPECT_TRUE(A0 == A2);
  EXPECT_FALSE(A0 == B);
}

TEST(Feature, lt) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("a"));
  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("aa"));
  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("b"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("aa"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("b"));
  EXPECT_LT(*FM->getFeature("aa"), *FM->getFeature("b"));
}

TEST(Feature, gt) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_GT(*FM->getFeature("a"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("aa"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("aa"), *FM->getFeature("a"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("a"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("aa"));
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

} // namespace vara::feature

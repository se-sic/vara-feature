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

TEST(Feature, disjunct) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"}});
  assert(FM);
  BinaryFeature B("B");

  EXPECT_NE(*FM->getFeature("a"), B);
  EXPECT_LT(*FM->getFeature("a"), B);
  EXPECT_GT(B, *FM->getFeature("a"));
}

TEST(Feature, ltEqual) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"}});
  assert(FM);

  EXPECT_FALSE(*FM->getFeature("a") < *FM->getFeature("a"));
}

/// root(a, b)
TEST(Feature, ltSimple) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("a"));
  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("b"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("b"));
}

/// root(a(aa), b)
TEST(Feature, ltSubtreeAA) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("aa"));
  EXPECT_LT(*FM->getFeature("aa"), *FM->getFeature("b"));
}

/// root(a(aa, ab), b)
TEST(Feature, ltSubtreeAB) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "b"}, {"root", "a"}, {"a", "ab"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("aa"), *FM->getFeature("ab"));
  EXPECT_LT(*FM->getFeature("ab"), *FM->getFeature("b"));
}

/// root(a(aa, ab), b(ba))
TEST(Feature, ltSubtreeBA) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "b"}, {"root", "a"}, {"b", "ba"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("b"), *FM->getFeature("ba"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("ba"));
}

/// root(a(aa, ab), b(ba, bb))
TEST(Feature, ltSubtreeBB) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"b", "bb"}, {"root", "b"}, {"root", "a"}, {"b", "ba"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("bb"));
  EXPECT_LT(*FM->getFeature("ba"), *FM->getFeature("bb"));
}

TEST(Feature, gtEqual) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"}});
  assert(FM);

  EXPECT_FALSE(*FM->getFeature("a") > *FM->getFeature("a"));
}

TEST(Feature, gtSimple) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_GT(*FM->getFeature("a"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("a"));
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

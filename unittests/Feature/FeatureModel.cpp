#include <vara/Feature/FeatureModel.h>

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModel, build) {
  FeatureModelBuilder B;

  B.addParent("a", "root")->makeFeature<BinaryFeature>("a", true);

  EXPECT_TRUE(B.buildFeatureModel());
}

TEST(FeatureModel, size) {
  FeatureModelBuilder B;

  B.addParent("a", "root")->makeFeature<BinaryFeature>("a", true);

  EXPECT_EQ(2, B.buildFeatureModel()->size());
}

TEST(FeatureModel, addFeature) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "aba"}, {"root", "a"}});
  assert(FM);

  FM->addFeature(std::make_unique<BinaryFeature>(
      "ab", false, std::nullopt, FM->getFeature("a"),
      Feature::NodeSetType({FM->getFeature("aba")})));

  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("ab"));
  EXPECT_GT(*FM->getFeature("aba"), *FM->getFeature("ab"));
  EXPECT_EQ(*FM->getFeature("aba")->getParentFeature(), *FM->getFeature("ab"));
}

TEST(FeatureModel, newRoot) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "b"}, {"root", "a"}});
  assert(FM);

  FM->addFeature(std::make_unique<BinaryFeature>(
      "new_root", false, std::nullopt, nullptr,
      Feature::NodeSetType({FM->getFeature("root")})));

  EXPECT_TRUE(FM->getFeature("new_root")->isRoot());
  EXPECT_FALSE(FM->getFeature("root")->isRoot());
  EXPECT_LT(*FM->getFeature("new_root"), *FM->getFeature("root"));
  EXPECT_LT(*FM->getFeature("new_root"), *FM->getFeature("a"));
  EXPECT_LT(*FM->getFeature("new_root"), *FM->getFeature("b"));
  EXPECT_EQ(*FM->getFeature("new_root"), **FM->begin());
}

TEST(FeatureModel, iter) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"a", "aa"},
                                                           {"a", "ab"},
                                                           {"b", "bb"},
                                                           {"root", "b"},
                                                           {"root", "a"},
                                                           {"b", "ba"}});
  assert(FM);

  auto Iter = FM->begin();
  EXPECT_EQ(**Iter++, *FM->getFeature("root"));
  EXPECT_EQ(**Iter++, *FM->getFeature("a"));
  EXPECT_EQ(**Iter++, *FM->getFeature("aa"));
  EXPECT_EQ(**Iter++, *FM->getFeature("ab"));
  EXPECT_EQ(**Iter++, *FM->getFeature("b"));
  EXPECT_EQ(**Iter++, *FM->getFeature("ba"));
  EXPECT_EQ(**Iter++, *FM->getFeature("bb"));
  EXPECT_EQ(Iter, FM->end());
}

TEST(FeatureModel, disjunct) {
  auto FMA = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"}});
  auto FMB = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "b"}});
  assert(FMA && FMB);

  EXPECT_NE(*FMA->getFeature("a"), *FMB->getFeature("b"));
  EXPECT_LT(*FMA->getFeature("a"), *FMB->getFeature("b"));
  EXPECT_GT(*FMB->getFeature("b"), *FMA->getFeature("a"));
}

TEST(FeatureModel, disjunctRaw) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"}});
  assert(FM);
  BinaryFeature B("B");

  EXPECT_NE(*FM->getFeature("a"), B);
  EXPECT_LT(*FM->getFeature("a"), B);
  EXPECT_GT(B, *FM->getFeature("a"));
}

TEST(FeatureModel, ltSameName) {
  auto FMA = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"}});
  auto FMB = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"}});
  assert(FMA && FMB);

  EXPECT_EQ(*FMA->getFeature("a"), *FMB->getFeature("a"));
  EXPECT_FALSE(*FMA->getFeature("a") < *FMB->getFeature("a"));
  EXPECT_FALSE(*FMB->getFeature("a") < *FMA->getFeature("a"));
}

TEST(FeatureModel, ltEqual) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"}});
  assert(FM);

  EXPECT_FALSE(*FM->getFeature("a") < *FM->getFeature("a"));
}

/// root(a, b)
TEST(FeatureModel, ltSimple) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("a"));
  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("b"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("b"));
}

/// root(a, b, c)
TEST(FeatureModel, ternary) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "b"}, {"root", "a"}, {"root", "c"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("b"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("c"));
  EXPECT_LT(*FM->getFeature("b"), *FM->getFeature("c"));
}

/// root(a(aa), b)
TEST(FeatureModel, ltSubtreeAA) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("aa"));
  EXPECT_LT(*FM->getFeature("aa"), *FM->getFeature("b"));
}

/// root(a(aa, ab), b)
TEST(FeatureModel, ltSubtreeAB) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "b"}, {"root", "a"}, {"a", "ab"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("aa"), *FM->getFeature("ab"));
  EXPECT_LT(*FM->getFeature("ab"), *FM->getFeature("b"));
}

/// root(a(aa, ab), b(ba))
TEST(FeatureModel, ltSubtreeBA) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"a", "ab"}, {"root", "b"}, {"root", "a"}, {"b", "ba"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("b"), *FM->getFeature("ba"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("ba"));
}

/// root(a(aa, ab), b(ba, bb))
TEST(FeatureModel, ltSubtreeBB) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"a", "aa"},
                                                           {"a", "ab"},
                                                           {"b", "bb"},
                                                           {"root", "b"},
                                                           {"root", "a"},
                                                           {"b", "ba"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("bb"));
  EXPECT_LT(*FM->getFeature("ba"), *FM->getFeature("bb"));
}

TEST(FeatureModel, gtEqual) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"}});
  assert(FM);

  EXPECT_FALSE(*FM->getFeature("a") > *FM->getFeature("a"));
}

TEST(FeatureModel, gtSimple) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_GT(*FM->getFeature("a"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("a"));
}
} // namespace vara::feature

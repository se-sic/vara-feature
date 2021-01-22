#include "llvm/ADT/SetVector.h"
#include <vara/Feature/FeatureModel.h>

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModel, build) {
  FeatureModelBuilder B;

  B.addParent("a", "root")->makeFeature<BinaryFeature>("a", true);

  EXPECT_TRUE(B.buildFeatureModel());
}

TEST(FeatureModel, cloneUnique) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "aba"}, {"root", "a"}});
  assert(FM);

  auto Clone = FM->clone();

  assert(Clone);
  for (const auto &Feature : FM->features()) {
    EXPECT_NE(Clone->getFeature(Feature->getName()), Feature);
  }
}

TEST(FeatureModel, cloneRoot) {
  FeatureModelBuilder B;
  BinaryFeature A("a");
  B.addFeature(A);
  B.setRoot("a");
  auto FM = B.buildFeatureModel();
  assert(FM);

  auto Clone = FM->clone();
  assert(Clone);
  FM.reset();

  // NOLINTNEXTLINE
  ASSERT_EXIT(EXPECT_TRUE(FM->getFeature("a")->isRoot()),
              testing::KilledBySignal(SIGSEGV), ".*");
  EXPECT_TRUE(Clone->getFeature("a")->isRoot());
}

TEST(FeatureModel, cloneRelationship) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  BinaryFeature AB("ab");
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  CS.insert(&AB);
  B.addParent("aa", "root")->addFeature(AA);
  B.addParent("ab", "root")->addFeature(AB);
  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, {"aa", "ab"},
                        "root");
  auto FM = B.buildFeatureModel();
  assert(FM);

  auto Clone = FM->clone();
  assert(Clone);
  FM.reset();

  EXPECT_FALSE(Clone->getRoot()->getChildren<Relationship>().empty());
  EXPECT_TRUE(llvm::isa<Relationship>(Clone->getFeature("aa")->getParent()));
  EXPECT_TRUE(llvm::isa<Relationship>(Clone->getFeature("ab")->getParent()));
}

TEST(FeatureModel, cloneConstraint) {
  FeatureModelBuilder B;
  BinaryFeature A("a");
  B.addFeature(A);
  B.addConstraint(std::make_unique<PrimaryFeatureConstraint>(&A));
  auto FM = B.buildFeatureModel();
  assert(FM);

  auto Clone = FM->clone();
  assert(Clone);
  auto *Deleted = *FM->getFeature("a")->constraints().begin();
  FM.reset();

  // NOLINTNEXTLINE
  ASSERT_EXIT(EXPECT_TRUE(Deleted->clone()), testing::KilledBySignal(SIGSEGV),
              ".*");
  EXPECT_TRUE((*Clone->getFeature("a")->constraints().begin())->clone());
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
  auto CS = Feature::NodeSetType();
  CS.insert(FM->getFeature("aba"));

  FM->addFeature(std::make_unique<BinaryFeature>(
      "ab", false, std::vector<FeatureSourceRange>(), FM->getFeature("a"), CS));

  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("ab"));
  EXPECT_GT(*FM->getFeature("aba"), *FM->getFeature("ab"));
  EXPECT_EQ(*FM->getFeature("aba")->getParentFeature(), *FM->getFeature("ab"));
}

TEST(FeatureModel, newRoot) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "b"}, {"root", "a"}});
  assert(FM);
  auto CS = Feature::NodeSetType();
  CS.insert(FM->getFeature("root"));

  FM->addFeature(std::make_unique<BinaryFeature>(
      "new_root", false, std::vector<FeatureSourceRange>(), nullptr, CS));

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
  std::vector<Feature *> Expected = {
      FM->getFeature("bb"),  FM->getFeature("ba"), FM->getFeature("b"),
      FM->getFeature("ab"),  FM->getFeature("aa"), FM->getFeature("a"),
      FM->getFeature("root")};

  EXPECT_EQ(Expected.size(), FM->size());
  for (const auto *F : FM->features()) {
    EXPECT_EQ(*Expected.back(), *F);
    Expected.pop_back();
  }
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

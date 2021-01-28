#include "vara/Feature/FeatureModel.h"

#include "llvm/ADT/SetVector.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModel, build) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("a", true);

  EXPECT_TRUE(B.buildFeatureModel());
}

TEST(FeatureModel, cloneUnique) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
  B.makeFeature<BinaryFeature>("b");
  auto FM = B.buildFeatureModel();
  assert(FM);

  auto Clone = FM->clone();

  assert(Clone);
  for (const auto &Feature : FM->features()) {
    EXPECT_NE(Clone->getFeature(Feature->getName()), Feature);
  }
}

TEST(FeatureModel, cloneRoot) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.setRoot("a");
  auto FM = B.buildFeatureModel();
  assert(FM);

  auto Clone = FM->clone();
  assert(Clone);
  FM.reset();

  // The inner EXPECT_TRUE just handles the nodiscard of isRoot
  // NOLINTNEXTLINE
  ASSERT_EXIT(EXPECT_TRUE(FM->getFeature("a")->isRoot()),
              testing::KilledBySignal(SIGSEGV), ".*");
  EXPECT_TRUE(Clone->getFeature("a")->isRoot());
}

TEST(FeatureModel, cloneRelationship) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, {"a", "b"},
                        "root");
  auto FM = B.buildFeatureModel();
  assert(FM);

  auto Clone = FM->clone();
  assert(Clone);
  FM.reset();

  EXPECT_FALSE(Clone->getRoot()->getChildren<Relationship>().empty());
  EXPECT_TRUE(llvm::isa<Relationship>(Clone->getFeature("a")->getParent()));
  EXPECT_TRUE(llvm::isa<Relationship>(Clone->getFeature("b")->getParent()));
}

TEST(FeatureModel, cloneConstraint) {
  FeatureModelBuilder B;
  B.addConstraint(std::make_unique<PrimaryFeatureConstraint>(
      B.makeFeature<BinaryFeature>("a")));
  auto FM = B.buildFeatureModel();
  assert(FM);

  auto Clone = FM->clone();
  assert(Clone);
  auto *Deleted = *FM->getFeature("a")->constraints().begin();
  FM.reset();

  // The inner EXPECT_TRUE just handles the nodiscard of clone
  // NOLINTNEXTLINE
  ASSERT_EXIT(EXPECT_TRUE(Deleted->clone()), testing::KilledBySignal(SIGSEGV),
              ".*");
  EXPECT_TRUE((*Clone->getFeature("a")->constraints().begin())->clone());
}

TEST(FeatureModel, size) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("a", true);

  EXPECT_EQ(2, B.buildFeatureModel()->size());
}

class FeatureModelTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder B;
    B.makeFeature<BinaryFeature>("a");
    B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
    B.addEdge("a", "ab")->makeFeature<BinaryFeature>("ab");
    B.makeFeature<BinaryFeature>("b");
    B.addEdge("b", "ba")->makeFeature<BinaryFeature>("ba");
    B.addEdge("b", "bb")->makeFeature<BinaryFeature>("bb");
    B.makeFeature<BinaryFeature>("c");
    FM = B.buildFeatureModel();
    assert(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelTest, iter) {
  std::vector<Feature *> Expected = {
      FM->getFeature("c"), FM->getFeature("bb"),  FM->getFeature("ba"),
      FM->getFeature("b"), FM->getFeature("ab"),  FM->getFeature("aa"),
      FM->getFeature("a"), FM->getFeature("root")};

  EXPECT_EQ(Expected.size(), FM->size());
  for (const auto *F : FM->features()) {
    EXPECT_EQ(*Expected.back(), *F);
    Expected.pop_back();
  }
}

TEST_F(FeatureModelTest, disjunct) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  auto FN = B.buildFeatureModel();
  assert(FN);

  EXPECT_NE(*FM->getFeature("a"), *FN->getFeature("b"));
  EXPECT_LT(*FM->getFeature("a"), *FN->getFeature("b"));
  EXPECT_GT(*FN->getFeature("b"), *FM->getFeature("a"));
}

TEST_F(FeatureModelTest, disjunctRaw) {
  BinaryFeature F("B");

  EXPECT_NE(*FM->getFeature("a"), F);
  EXPECT_LT(*FM->getFeature("a"), F);
  EXPECT_GT(F, *FM->getFeature("a"));
}

TEST_F(FeatureModelTest, ltSameName) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  auto FN = B.buildFeatureModel();
  assert(FN);

  EXPECT_EQ(*FM->getFeature("a"), *FN->getFeature("a"));
  EXPECT_FALSE(*FM->getFeature("a") < *FN->getFeature("a"));
  EXPECT_FALSE(*FN->getFeature("a") < *FM->getFeature("a"));
}

TEST_F(FeatureModelTest, ltEqual) {
  EXPECT_FALSE(*FM->getFeature("a") < *FM->getFeature("a"));
}

/// root(a, b)
TEST_F(FeatureModelTest, ltSimple) {
  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("a"));
  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("b"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("b"));
}

/// root(a, b, c)
TEST_F(FeatureModelTest, ternary) {
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("b"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("c"));
  EXPECT_LT(*FM->getFeature("b"), *FM->getFeature("c"));
}

/// root(a(aa), b)
TEST_F(FeatureModelTest, ltSubtreeAA) {
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("aa"));
  EXPECT_LT(*FM->getFeature("aa"), *FM->getFeature("b"));
}

/// root(a(aa, ab), b)
TEST_F(FeatureModelTest, ltSubtreeAB) {
  EXPECT_LT(*FM->getFeature("aa"), *FM->getFeature("ab"));
  EXPECT_LT(*FM->getFeature("ab"), *FM->getFeature("b"));
}

/// root(a(aa, ab), b(ba))
TEST_F(FeatureModelTest, ltSubtreeBA) {
  EXPECT_LT(*FM->getFeature("b"), *FM->getFeature("ba"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("ba"));
}

/// root(a(aa, ab), b(ba, bb))
TEST_F(FeatureModelTest, ltSubtreeBB) {
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("bb"));
  EXPECT_LT(*FM->getFeature("ba"), *FM->getFeature("bb"));
}

TEST_F(FeatureModelTest, gtEqual) {
  EXPECT_FALSE(*FM->getFeature("a") > *FM->getFeature("a"));
}

TEST_F(FeatureModelTest, gtSimple) {
  EXPECT_GT(*FM->getFeature("a"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("a"));
}

//===----------------------------------------------------------------------===//
//                    FeatureModelConsistencyChecker Tests
//===----------------------------------------------------------------------===//

class FeatureModelConsistencyCheckerTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder B;
    B.makeFeature<BinaryFeature>("a");
    B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
    B.addEdge("a", "ab")->makeFeature<BinaryFeature>("ab");
    B.makeFeature<BinaryFeature>("b");
    B.addEdge("b", "ba")->makeFeature<BinaryFeature>("ba");
    B.addEdge("b", "bb")->makeFeature<BinaryFeature>("bb");
    B.makeFeature<BinaryFeature>("c");
    FM = B.buildFeatureModel();
    assert(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelConsistencyCheckerTest, NoChecksIsTrue) {
  EXPECT_TRUE(FeatureModelConsistencyChecker<>::isFeatureModelValid(*FM));
}

TEST_F(FeatureModelConsistencyCheckerTest, EveryFeatureRequiresParentValid) {
  EXPECT_TRUE(FeatureModelConsistencyChecker<
              EveryFeatureRequiresParent>::isFeatureModelValid(*FM));
}

TEST_F(FeatureModelConsistencyCheckerTest, EveryFeatureRequiresParentMissing) {
  // TODO: find save way to construct wrong FeatureModels
  // FM->deleteFeature("missing");

  FM->dump();

  // EXPECT_FALSE(FeatureModelConsistencyChecker<
  //             EveryFeatureRequiresParent>::isFeatureModelValid(*FM));
}

} // namespace vara::feature

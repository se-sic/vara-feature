#include "vara/Feature/FeatureModelBuilder.h"
#include "vara/Feature/FeatureModelTransaction.h"

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
  B.makeFeature<BinaryFeature>("aa")->addEdge("a", "aa");
  B.makeFeature<BinaryFeature>("b");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  auto Clone = FM->clone();

  ASSERT_TRUE(Clone);
  for (const auto &Feature : FM->features()) {
    EXPECT_NE(Clone->getFeature(Feature->getName()), Feature);
  }
}

TEST(FeatureModel, cloneRoot) {
  FeatureModelBuilder B;
  B.makeRoot("a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  auto Clone = FM->clone();
  ASSERT_TRUE(Clone);
  FM.reset();

  // The inner EXPECT_TRUE just handles the nodiscard of getParent
  // NOLINTNEXTLINE
  EXPECT_DEATH(EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a"))), ".*");
  EXPECT_TRUE(llvm::isa<RootFeature>(Clone->getFeature("a")));
}

TEST(FeatureModel, cloneRelationship) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("aa")->addEdge("a", "aa");
  B.makeFeature<BinaryFeature>("ab")->addEdge("a", "ab");
  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, "a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  auto Clone = FM->clone();
  ASSERT_TRUE(Clone);
  FM.reset();

  EXPECT_FALSE(Clone->getFeature("a")->getChildren<Relationship>().empty());
  EXPECT_TRUE(llvm::isa<Relationship>(Clone->getFeature("aa")->getParent()));
  EXPECT_TRUE(llvm::isa<Relationship>(Clone->getFeature("ab")->getParent()));
}

TEST(FeatureModel, cloneConstraint) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addConstraint(std::make_unique<PrimaryFeatureConstraint>(
      std::make_unique<BinaryFeature>("a")));
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  auto Clone = FM->clone();
  ASSERT_TRUE(Clone);
  auto *Deleted = *FM->getFeature("a")->constraints().begin();
  FM.reset();

  // The inner EXPECT_TRUE just handles the nodiscard of clone
  // NOLINTNEXTLINE
  EXPECT_DEATH(EXPECT_TRUE(Deleted->clone()), ".*");
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
    ASSERT_TRUE(FM);
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
  ASSERT_TRUE(FN);

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
  ASSERT_TRUE(FN);

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

TEST_F(FeatureModelTest, dfs) {
  std::vector<Feature *> Expected = {
      FM->getFeature("c"), FM->getFeature("bb"),  FM->getFeature("ba"),
      FM->getFeature("b"), FM->getFeature("ab"),  FM->getFeature("aa"),
      FM->getFeature("a"), FM->getFeature("root")};

  EXPECT_EQ(Expected.size(), FM->size());
  for (const auto *F : *FM) {
    EXPECT_STREQ(Expected.back()->getName().data(), F->getName().data());
    ASSERT_EQ(*Expected.back(), *F);
    Expected.pop_back();
  }
}

//===----------------------------------------------------------------------===//
//                    FeatureModelConsistencyChecker Tests
//===----------------------------------------------------------------------===//

class FeatureModelGetChildrenTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder B;
    B.makeFeature<BinaryFeature>("a");
    B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
    B.addEdge("a", "ab")->makeFeature<BinaryFeature>("ab");
    B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, "ab");
    B.addEdge("ab", "aba")->makeFeature<BinaryFeature>("aba");
    B.addEdge("ab", "abb")->makeFeature<BinaryFeature>("abb");
    B.makeFeature<BinaryFeature>("b");
    B.emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, "b");
    B.addEdge("b", "ba")->makeFeature<BinaryFeature>("ba");
    B.addEdge("b", "bb")->makeFeature<BinaryFeature>("bb");
    B.makeFeature<BinaryFeature>("c");
    B.addEdge("c", "ca")->makeFeature<NumericFeature>("ca", std::pair(1, 5));
    B.addEdge("c", "cb")->makeFeature<NumericFeature>("cb", std::pair(1, 5));
    FM = B.buildFeatureModel();
    ASSERT_TRUE(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

//===----------------------------------------------------------------------===//
//                    FeatureModelConsistencyChecker Tests
//===----------------------------------------------------------------------===//

class FeatureModelConsistencyCheckerTest
    : public ::testing::Test,
      protected detail::FeatureModelModification {
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
    ASSERT_TRUE(FM);
  }

  // Dummy method to fulfill the FeatureModelModification interface
  void exec(FeatureModel &_) override{};

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
  FeatureModelModification::removeParent(*FM->getFeature("a"));

  EXPECT_FALSE(FeatureModelConsistencyChecker<
               EveryFeatureRequiresParent>::isFeatureModelValid(*FM));
}

TEST_F(FeatureModelConsistencyCheckerTest, EveryParentNeedsFeatureAsAChild) {
  EXPECT_TRUE(FeatureModelConsistencyChecker<
              CheckFeatureParentChildRelationShip>::isFeatureModelValid(*FM));
}

TEST_F(FeatureModelConsistencyCheckerTest,
       EveryParentNeedsFeatureAsAChildMissing) {
  FeatureModelModification::removeEdge(*FM->getRoot(), *FM->getFeature("a"));

  EXPECT_FALSE(FeatureModelConsistencyChecker<
               CheckFeatureParentChildRelationShip>::isFeatureModelValid(*FM));
}

TEST_F(FeatureModelConsistencyCheckerTest,
       EveryFMNeedsOneRootOnlyOneRootPresent) {
  EXPECT_TRUE(
      FeatureModelConsistencyChecker<ExactlyOneRootNode>::isFeatureModelValid(
          *FM));
}

TEST_F(FeatureModelConsistencyCheckerTest, FMNoRoot) {
  FeatureModel FM;

  FeatureModelModification::removeFeature(FM, *FM.getRoot());

  EXPECT_EQ(FM.size(), 0);
  EXPECT_EQ(FM.begin(), FM.end());
  EXPECT_FALSE(
      FeatureModelConsistencyChecker<ExactlyOneRootNode>::isFeatureModelValid(
          FM));
}

TEST_F(FeatureModelConsistencyCheckerTest,
       EveryFMNeedsOneRootButMultiplePresent) {
  ASSERT_TRUE(FeatureModelModification::addFeature(
      *FM, std::make_unique<RootFeature>("z")));

  EXPECT_FALSE(
      FeatureModelConsistencyChecker<ExactlyOneRootNode>::isFeatureModelValid(
          *FM));
}

} // namespace vara::feature

#include "vara/Feature/Relationship.h"
#include "vara/Feature/FeatureModelBuilder.h"

#include "gtest/gtest.h"

namespace vara::feature {

class RelationshipTest : public ::testing::Test {
protected:
  void SetUp() override {
    B.makeFeature<NumericFeature>("a", std::vector<int>{1, 2, 3});
    B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
    B.addEdge("a", "ab")->makeFeature<BinaryFeature>("ab");
  }

  FeatureModelBuilder B;
};

TEST_F(RelationshipTest, orTree) {
  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, "a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  if (auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
      R) {
    EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_OR);
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("aa")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
  } else {
    FAIL();
  }
}

TEST_F(RelationshipTest, alternativeTree) {
  B.emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, "a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  if (auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
      R) {
    EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_ALTERNATIVE);
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("aa")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
  } else {
    FAIL();
  }
}

TEST(Relationship, outOfOrder) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addEdge("a", "aa");
  B.addEdge("a", "ac");
  B.addEdge("a", "ab");
  B.makeFeature<BinaryFeature>("ab", false);
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ac", false);

  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, "a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ac")));
  if (auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
      R) {
    EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_OR);
    EXPECT_TRUE(R->hasEdgeFrom(*FM->getFeature("a")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("aa")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ac")));
  } else {
    FAIL();
  }
}

TEST(Relationship, getChildren) {
  FeatureModelBuilder B;
  B.makeFeature<NumericFeature>("a", std::vector<int>{1, 2, 3});
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
  B.addEdge("a", "ab")->makeFeature<BinaryFeature>("ab");

  B.emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, "a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(FM->getFeature("a")->getChildren<Relationship>(0).size(), 0);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Relationship>(1).size(), 1);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Relationship>(2).size(), 1);
}

} // namespace vara::feature

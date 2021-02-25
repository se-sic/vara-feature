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
  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, {"aa", "ab"},
                        "a");
  auto FM = B.buildFeatureModel();
  assert(FM);

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
  B.emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE,
                        {"aa", "ab"}, "a");
  auto FM = B.buildFeatureModel();
  assert(FM);

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
  B.addEdge("a", "ab");
  B.addEdge("a", "ac");
  B.addEdge("a", "ad");
  B.addEdge("a", "ae");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ab", false);
  B.makeFeature<BinaryFeature>("ac", false);
  B.makeFeature<BinaryFeature>("ad", false);
  B.makeFeature<BinaryFeature>("ae", false);

  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, {"ad", "ab"},
                        "a");
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ac")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ad")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ae")));
}

TEST(Relationship, diffParents) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  B.addEdge("a", "aa");
  B.addEdge("b", "ba");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ba", false);

  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, {"ba", "aa"},
                        "a");

  EXPECT_FALSE(B.buildFeatureModel());
}

} // namespace vara::feature

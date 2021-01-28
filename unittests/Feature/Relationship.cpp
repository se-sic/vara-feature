#include "vara/Feature/Relationship.h"
#include "vara/Feature/FeatureModel.h"

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

  auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
  assert(R);

  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_OR);
  EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
}

TEST_F(RelationshipTest, alternativeTree) {
  B.emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE,
                        {"aa", "ab"}, "a");
  auto FM = B.buildFeatureModel();
  assert(FM);

  auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
  assert(R);

  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_ALTERNATIVE);
  EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
}
} // namespace vara::feature

#include "vara/Feature/Relationship.h"
#include "vara/Feature/FeatureModel.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(Relationship, orTree) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  BinaryFeature AB("ab");
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  CS.insert(&AB);
  NumericFeature A("a", std::vector<int>{1, 2, 3}, false, {}, nullptr,
                   CS);
  B.addFeature(A);
  B.addParent("aa", "a")->addFeature(AA);
  B.addParent("ab", "a")->addFeature(AB);

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

TEST(Relationship, alternativeTree) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  BinaryFeature AB("ab");
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  CS.insert(&AB);
  NumericFeature A("a", std::vector<int>{1, 2, 3}, false, {}, nullptr,
                   CS);
  B.addFeature(A);
  B.addParent("aa", "a")->addFeature(AA);
  B.addParent("ab", "a")->addFeature(AB);

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

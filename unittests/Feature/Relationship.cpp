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
  BinaryFeature A("a", false, {}, nullptr, CS);
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
  BinaryFeature A("a", false, {}, nullptr, CS);
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

TEST(Relationship, outOfOrder) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  BinaryFeature AB("ab");
  BinaryFeature AC("ac");
  BinaryFeature AD("ad");
  BinaryFeature AE("ae");
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  CS.insert(&AB);
  CS.insert(&AC);
  CS.insert(&AD);
  BinaryFeature A("a", false, {}, nullptr, CS);
  B.addFeature(A);
  B.addParent("aa", "a")->addFeature(AA);
  B.addParent("ab", "a")->addFeature(AB);
  B.addParent("ac", "a")->addFeature(AC);
  B.addParent("ad", "a")->addFeature(AD);
  B.addParent("ae", "a")->addFeature(AE);

  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, {"ad", "ab"},
                        "a");
  auto FM = B.buildFeatureModel();
  assert(FM);
  auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
  assert(R);

  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ac")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ad")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ae")));
  EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_OR);
  EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ad")));
}

TEST(Relationship, diffParents) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  BinaryFeature BA("ba");
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  auto BS = Feature::NodeSetType();
  BS.insert(&BA);
  BinaryFeature A("a", false, {}, nullptr, CS);
  BinaryFeature C("b", false, {}, nullptr, BS);
  B.addFeature(A);
  B.addFeature(C);
  B.addParent("aa", "a")->addFeature(AA);
  B.addParent("ba", "b")->addFeature(BA);

  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, {"ba", "aa"},
                        "a");

  EXPECT_FALSE(B.buildFeatureModel());
}

} // namespace vara::feature

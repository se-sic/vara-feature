#include "vara/Feature/FeatureModelBuilder.h"

#include "gtest/gtest.h"

namespace vara::feature {

class FeatureTreeNodeTest : public ::testing::Test {
protected:
  void SetUp() override {
    B.makeFeature<NumericFeature>("a", std::vector<int>{1, 2, 3});
    B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
    B.addEdge("a", "ab")->makeFeature<BinaryFeature>("ab");
  }

  FeatureModelBuilder B;
};

TEST_F(FeatureTreeNodeTest, getChildren) {
  B.emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, "a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(FM->getFeature("a")->getChildren().size(), 1);
  EXPECT_EQ((*FM->getFeature("a")->getChildren().begin())->getChildren().size(),
            2);
}

TEST_F(FeatureTreeNodeTest, getChildrenFeature) {
  B.emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, "a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);
  ASSERT_EQ(FM->getFeature("a")->getChildren<Feature>().size(), 2);

  EXPECT_EQ(FM->getFeature("a")->getChildren<Feature>(0).size(), 0);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Feature>(1).size(), 0);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Feature>(2).size(), 2);
}

TEST_F(FeatureTreeNodeTest, getChildrenRelationship) {
  B.emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, "a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(FM->getFeature("a")->getChildren<Relationship>(0).size(), 0);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Relationship>(1).size(), 1);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Relationship>(2).size(), 1);
}

TEST_F(FeatureTreeNodeTest, getChildrenNone) {
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);
  ASSERT_EQ(FM->getFeature("a")->getChildren<Feature>().size(), 2);

  EXPECT_EQ(FM->getFeature("a")->getChildren<Relationship>().size(), 0);
  EXPECT_EQ(FM->getFeature("a")->getChildren<Relationship>(42).size(), 0);
}

} // namespace vara::feature

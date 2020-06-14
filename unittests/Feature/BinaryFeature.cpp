#include "vara/Feature/Feature.h"
#include <vara/Feature/FeatureModel.h>

#include "gtest/gtest.h"

namespace vara::feature {
TEST(BinaryFeature, basicAccessors) {
  BinaryFeature A("A", true);

  EXPECT_EQ("A", A.getName());
  EXPECT_TRUE(A.isOptional());
  EXPECT_TRUE(A.isRoot());
}

TEST(BinaryFeature, BinaryFeatureRoot) {
  auto B = FeatureModel::FeatureModelBuilder();

  B.addFeature("F");
  B.setRoot("F");

  auto FM = FeatureModel::FeatureModelBuilder().buildFeatureModel();

  EXPECT_TRUE(FM->getFeature("F")->isRoot());
  EXPECT_EQ(FM->getFeature("F"), FM->getRoot());
}

TEST(BinaryFeature, BinaryFeatureChildren) {
  auto FM = FeatureModel::FeatureModelBuilder().buildSimpleFeatureModel(
      {{"F", "A"}, {"root", {"F"}}});

  EXPECT_EQ(std::distance(FM->getFeature("F")->children_begin(),
                          FM->getFeature("F")->children_end()),
            1);
  EXPECT_EQ("A", (*FM->getFeature("F")->children_begin())->getName());
}

TEST(BinaryFeature, BinaryFeatureExclude) {
  auto B = FeatureModel::FeatureModelBuilder();

  B.addFeature("F");
  B.addFeature("G");
  B.addExclude("F", "G");

  auto FM = FeatureModel::FeatureModelBuilder().buildFeatureModel();

  EXPECT_EQ(std::distance(FM->getFeature("F")->excludes_begin(),
                          FM->getFeature("F")->excludes_end()),
            1);
  EXPECT_EQ("G", (*FM->getFeature("F")->excludes_begin())->getName());
}

TEST(BinaryFeature, BinaryFeatureImplications) {
  auto B = FeatureModel::FeatureModelBuilder();

  B.addFeature("F");
  B.addFeature("G");
  B.addImplication("F", "G");

  auto FM = FeatureModel::FeatureModelBuilder().buildFeatureModel();

  EXPECT_EQ(std::distance(FM->getFeature("F")->implications_begin(),
                          FM->getFeature("F")->implications_end()),
            1);
  EXPECT_EQ("G", (*FM->getFeature("F")->implications_begin())->getName());
}

TEST(BinaryFeature, BinaryFeatureAlternatives) {
  auto B = FeatureModel::FeatureModelBuilder();

  B.addFeature("F");
  B.addFeature("G");
  B.addAlternative("F", "G");

  auto FM = FeatureModel::FeatureModelBuilder().buildFeatureModel();

  EXPECT_EQ(std::distance(FM->getFeature("F")->alternatives_begin(),
                          FM->getFeature("F")->alternatives_end()),
            1);
  EXPECT_EQ("G", (*FM->getFeature("F")->alternatives_begin())->getName());
}
} // namespace vara::feature

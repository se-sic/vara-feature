#include "vara/Feature/Feature.h"
#include <vara/Feature/FeatureModel.h>

#include "llvm/Support/Casting.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace vara::feature {
TEST(NumericFeature, NumericFeatureBasics) {
  NumericFeature A("A", std::pair<int, int>(0, 1), true);

  EXPECT_EQ("A", A.getName());
  EXPECT_TRUE(A.isOptional());
  EXPECT_TRUE(A.isRoot());
}

TEST(NumericFeature, isa) {
  NumericFeature A("A", std::pair<int, int>(0, 1));

  EXPECT_TRUE(llvm::isa<NumericFeature>(A));
  EXPECT_FALSE(llvm::isa<BinaryFeature>(A));
}

TEST(NumericFeature, NumericFeaturePair) {
  NumericFeature A("A", std::pair<int, int>(0, 1));

  EXPECT_TRUE((std::holds_alternative<std::pair<int, int>>(A.getValues())));
  EXPECT_EQ((std::get<std::pair<int, int>>(A.getValues())).first, 0);
  EXPECT_EQ((std::get<std::pair<int, int>>(A.getValues())).second, 1);
}

TEST(NumericFeature, NumericFeatureVector) {
  NumericFeature A("A", std::vector<int>{0, 1, 2, 3});

  EXPECT_TRUE(std::holds_alternative<std::vector<int>>(A.getValues()));
  EXPECT_THAT(std::get<std::vector<int>>(A.getValues()),
              testing::ElementsAre(0, 1, 2, 3));
}

TEST(NumericFeature, NumericFeatureRoot) {
  auto B = FeatureModelBuilder();

  B.makeFeature<NumericFeature>("F", std::pair<int, int>(0, 1));
  B.setRoot("F");

  auto FM = B.buildFeatureModel();

  EXPECT_TRUE(FM->getFeature("F")->isRoot());
  EXPECT_EQ(FM->getFeature("F"), FM->getRoot());
}

TEST(NumericFeature, NumericFeatureChildren) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"F", "A"}}, {{"root", {"F", std::pair<int, int>(0, 1)}}});

  EXPECT_EQ(
      std::distance(FM->getFeature("F")->begin(), FM->getFeature("F")->end()),
      1);
  EXPECT_EQ(
      "A", llvm::dyn_cast<vara::feature::Feature>(*FM->getFeature("F")->begin())
               ->getName());
}

// TODO(s9latimm): Refactor with new Constraints representation
// TEST(NumericFeature, NumericFeatureExclude) {
//  auto B = FeatureModelBuilder();
//
//  B.makeFeature<NumericFeature>("F", std::pair<int, int>(0, 1));
//  B.makeFeature<NumericFeature>("G", std::pair<int, int>(0, 1));
//  B.addExclude("F", "G");
//
//  auto FM = B.buildFeatureModel();
//
//  EXPECT_EQ(std::distance(FM->getFeature("F")->excludes_begin(),
//                          FM->getFeature("F")->excludes_end()),
//            1);
//  EXPECT_EQ("G", (*FM->getFeature("F")->excludes_begin())->getName());
//}

// TODO(s9latimm): Refactor with new Constraints representation
// TEST(NumericFeature, NumericFeatureImplications) {
//  auto B = FeatureModelBuilder();
//
//  B.makeFeature<NumericFeature>("F", std::pair<int, int>(0, 1));
//  B.makeFeature<NumericFeature>("G", std::pair<int, int>(0, 1));
//  B.addConstraint({{"F", false}, {"G", true}});
//
//  auto FM = B.buildFeatureModel();
//
//  EXPECT_EQ(std::distance(FM->getFeature("F")->implications_begin(),
//                          FM->getFeature("F")->implications_end()),
//            1);
//  EXPECT_EQ("G", (*FM->getFeature("F")->implications_begin())->getName());
//}

// TODO(s9latimm): Refactor with new Constraints representation
// TEST(NumericFeature, NumericFeatureAlternatives) {
//  auto B = FeatureModelBuilder();
//
//  B.makeFeature<NumericFeature>("F", std::pair<int, int>(0, 1));
//  B.makeFeature<NumericFeature>("G", std::pair<int, int>(0, 1));
//  B.addConstraint({{"F", true}, {"G", true}});
//
//  auto FM = B.buildFeatureModel();
//
//  EXPECT_EQ(std::distance(FM->getFeature("F")->alternatives_begin(),
//                          FM->getFeature("F")->alternatives_end()),
//            1);
//  EXPECT_EQ("G", (*FM->getFeature("F")->alternatives_begin())->getName());
//}
} // namespace vara::feature

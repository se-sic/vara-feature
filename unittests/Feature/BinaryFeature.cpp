#include "vara/Feature/Feature.h"
#include <vara/Feature/FeatureModel.h>

#include "llvm/Support/Casting.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(BinaryFeature, basicAccessors) {
  BinaryFeature A("A", true);

  EXPECT_EQ("A", A.getName());
  EXPECT_TRUE(A.isOptional());
  EXPECT_TRUE(A.isRoot());
}

TEST(BinaryFeature, isa) {
  BinaryFeature A("A", true);

  EXPECT_TRUE(llvm::isa<BinaryFeature>(A));
  EXPECT_FALSE(llvm::isa<NumericFeature>(A));
}

TEST(BinaryFeature, BinaryFeatureRoot) {
  auto B = FeatureModelBuilder();

  B.makeFeature<BinaryFeature>("F");
  B.setRoot("F");

  auto FM = B.buildFeatureModel();

  EXPECT_TRUE(FM->getFeature("F")->isRoot());
  EXPECT_EQ(FM->getFeature("F"), FM->getRoot());
}

TEST(BinaryFeature, BinaryFeatureChildren) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"F", "A"}, {"root", {"F"}}});

  EXPECT_EQ(
      std::distance(FM->getFeature("F")->begin(), FM->getFeature("F")->end()),
      1);
  //  EXPECT_EQ("A", (*FM->getFeature("F")->begin())->getName());
}

// TEST(BinaryFeature, BinaryFeatureExclude) {
//  auto B = FeatureModelBuilder();
//
//  B.makeFeature<BinaryFeature>("F");
//  B.makeFeature<BinaryFeature>("G");
//  B.addExclude("F", "G");
//
//  auto FM = B.buildFeatureModel();
//
//  EXPECT_EQ(std::distance(FM->getFeature("F")->excludes_begin(),
//                          FM->getFeature("F")->excludes_end()),
//            1);
//  EXPECT_EQ("G", (*FM->getFeature("F")->excludes_begin())->getName());
//}

// TEST(BinaryFeature, BinaryFeatureImplications) {
//  auto B = FeatureModelBuilder();
//
//  B.makeFeature<BinaryFeature>("F");
//  B.makeFeature<BinaryFeature>("G");
//  B.addConstraint({{"F", false}, {"G", true}});
//
//  auto FM = B.buildFeatureModel();
//
//  EXPECT_EQ(std::distance(FM->getFeature("F")->implications_begin(),
//                          FM->getFeature("F")->implications_end()),
//            1);
//  EXPECT_EQ("G", (*FM->getFeature("F")->implications_begin())->getName());
//}

// TEST(BinaryFeature, BinaryFeatureAlternatives) {
//  auto B = FeatureModelBuilder();
//
//  B.makeFeature<BinaryFeature>("F");
//  B.makeFeature<BinaryFeature>("G");
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

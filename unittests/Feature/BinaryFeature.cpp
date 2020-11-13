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

  auto *F =
      llvm::dyn_cast<vara::feature::Feature>(*FM->getFeature("F")->begin());
  assert(F);
  EXPECT_EQ("A", F->getName());
}
} // namespace vara::feature

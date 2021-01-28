#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"

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
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");

  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(
      std::distance(FM->getFeature("a")->begin(), FM->getFeature("a")->end()),
      1);
  if (auto *F =
          llvm::dyn_cast<vara::feature::Feature>(*FM->getFeature("a")->begin());
      F) {
    EXPECT_EQ("aa", F->getName());
  } else {
    FAIL();
  }
}

} // namespace vara::feature

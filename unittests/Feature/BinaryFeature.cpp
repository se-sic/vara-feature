#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"

#include "llvm/Support/Casting.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(BinaryFeature, basicAccessors) {
  BinaryFeature A("A", true);

  EXPECT_EQ("A", A.getName());
  EXPECT_TRUE(A.isOptional());
  EXPECT_FALSE(A.getParent());
}

TEST(BinaryFeature, isa) {
  BinaryFeature A("A", true);

  EXPECT_TRUE(llvm::isa<BinaryFeature>(A));
  EXPECT_FALSE(llvm::isa<NumericFeature>(A));
  EXPECT_FALSE(llvm::isa<RootFeature>(A));
}

TEST(BinaryFeature, BinaryFeatureRoot) {
  auto B = FeatureModelBuilder();

  B.makeFeature<BinaryFeature>("F");
  B.setRootName("F");

  EXPECT_FALSE(B.buildFeatureModel());
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
  if (auto *F = llvm::dyn_cast<Feature>(*FM->getFeature("a")->begin())) {
    EXPECT_EQ("aa", F->getName());
  } else {
    FAIL();
  }
}

} // namespace vara::feature

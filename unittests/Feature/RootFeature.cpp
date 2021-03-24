#include "vara/Feature/FeatureModelBuilder.h"

#include "llvm/Support/Casting.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(RootFeature, basicAccessors) {
  RootFeature A("A");

  EXPECT_EQ("A", A.getName());
  EXPECT_FALSE(A.isOptional());
  EXPECT_FALSE(A.getParent());
}

TEST(RootFeature, isa) {
  RootFeature A("A");

  EXPECT_TRUE(llvm::isa<RootFeature>(A));
  EXPECT_FALSE(llvm::isa<BinaryFeature>(A));
  EXPECT_FALSE(llvm::isa<NumericFeature>(A));
}

TEST(RootFeature, RootFeatureRoot) {
  auto B = FeatureModelBuilder();

  B.makeRoot("F");
  auto FM = B.buildFeatureModel();

  EXPECT_TRUE(FM);
  EXPECT_TRUE(FM->getRoot());
  EXPECT_EQ("F", FM->getRoot()->getName());
}

TEST(RootFeature, RootFeatureChildren) {
  FeatureModelBuilder B;
  B.makeRoot("a");
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");

  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(
      std::distance(FM->getFeature("a")->begin(), FM->getFeature("a")->end()),
      1);
  if (auto *F = llvm::dyn_cast<Feature>(*FM->getFeature("a")->begin())) {
    EXPECT_EQ("aa", F->getName());
  } else {
    FAIL();
  }
}

TEST(RootFeature, RootFeatureChange) {
  auto B = FeatureModelBuilder();

  B.makeFeature<BinaryFeature>("A");
  B.makeRoot("F");
  auto FM = B.buildFeatureModel();

  EXPECT_TRUE(FM);
  EXPECT_TRUE(FM->getRoot());
  EXPECT_EQ("F", FM->getRoot()->getName());
  EXPECT_EQ("F", FM->getFeature("A")->getParentFeature()->getName());
}

} // namespace vara::feature

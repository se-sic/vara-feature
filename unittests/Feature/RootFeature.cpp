#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"

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

  B.makeFeature<RootFeature>("F");
  B.setRootName("F");
  auto FM = B.buildFeatureModel();

  EXPECT_TRUE(FM);
  EXPECT_EQ("F", FM->getRoot()->getName());
}

TEST(RootFeature, RootFeatureChildren) {
  FeatureModelBuilder B;
  B.setRootName("a")->makeFeature<RootFeature>("a");
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");

  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(
      std::distance(FM->getFeature("a")->begin(), FM->getFeature("a")->end()),
      1);
  Feature *FirstChild = llvm::dyn_cast<Feature>(*FM->getFeature("a")->begin());
  EXPECT_NE(FirstChild, nullptr);
  EXPECT_EQ("aa", F->getName());
}

} // namespace vara::feature

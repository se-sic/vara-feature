#include "vara/Feature/FeatureModelBuilder.h"

#include "llvm/Support/Casting.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace vara::feature {
TEST(NumericFeature, NumericFeatureBasics) {
  NumericFeature A("A", std::pair<int, int>(0, 1), true);

  EXPECT_EQ("A", A.getName());
  EXPECT_TRUE(A.isOptional());
  EXPECT_FALSE(A.getParent());
}

TEST(NumericFeature, isa) {
  NumericFeature A("A", std::pair<int, int>(0, 1));

  EXPECT_TRUE(llvm::isa<NumericFeature>(A));
  EXPECT_FALSE(llvm::isa<BinaryFeature>(A));
  EXPECT_FALSE(llvm::isa<RootFeature>(A));
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
  B.makeRoot("F");

  // TODO(s9latimm): As we currently have no error handling for failing
  //  transactions, the second modification (makeRoot) will not succeed in
  //  changing the FM, but the build will still complete.
  // EXPECT_FALSE(B.buildFeatureModel());
}

TEST(NumericFeature, NumericFeatureChildren) {
  FeatureModelBuilder B;
  B.makeFeature<NumericFeature>("a", std::pair<int, int>(0, 1));
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

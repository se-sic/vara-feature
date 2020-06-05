#include "vara/Feature/Feature.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace vara::feature {
TEST(NumericFeature, NumericFeatureBasics) {
  NumericFeature A("A", true, std::pair<int, int>(0, 1));

  EXPECT_EQ("A", A.getName());
  EXPECT_TRUE(A.isOptional());
  EXPECT_TRUE(A.isRoot());
}

TEST(NumericFeature, NumericFeaturePair) {
  NumericFeature A("A", false, std::pair<int, int>(0, 1));

  EXPECT_TRUE((std::holds_alternative<std::pair<int, int>>(A.getValues())));
  EXPECT_EQ((std::get<std::pair<int, int>>(A.getValues())).first, 0);
  EXPECT_EQ((std::get<std::pair<int, int>>(A.getValues())).second, 1);
}

TEST(NumericFeature, NumericFeatureVector) {
  NumericFeature A("A", false, std::vector<int>{0, 1, 2, 3});

  EXPECT_TRUE(std::holds_alternative<std::vector<int>>(A.getValues()));
  EXPECT_THAT(std::get<std::vector<int>>(A.getValues()),
              testing::ElementsAre(0, 1, 2, 3));
}

// TEST(NumericFeature, NumericFeatureRoot) {
//  NumericFeature A("A", false, std::pair<int, int>(0, 1));
//  NumericFeature B("B", false, std::pair<int, int>(0, 1));
//
//  B.addParent(&A);
//
//  EXPECT_TRUE(A.isRoot());
//  EXPECT_FALSE(B.isRoot());
//  EXPECT_EQ("A", B.getParent()->getName());
//}
//
// TEST(NumericFeature, NumericFeatureChildren) {
//  NumericFeature A("A", false, std::pair<int, int>(0, 1));
//  NumericFeature B("B", false, std::pair<int, int>(0, 1));
//
//  B.addChild(&A);
//
//  EXPECT_EQ(std::distance(B.children().begin(), B.children().end()), 1);
//  EXPECT_EQ("A", (*B.children().begin())->getName());
//}
//
// TEST(NumericFeature, NumericFeatureExclude) {
//  NumericFeature A("A", false, std::pair<int, int>(0, 1));
//  NumericFeature B("B", false, std::pair<int, int>(0, 1));
//
//  B.addExclude(&A);
//
//  EXPECT_EQ(std::distance(B.excludes_begin(), B.excludes_end()), 1);
//  EXPECT_EQ("A", (*B.excludes().begin())->getName());
//}
//
// TEST(NumericFeature, NumericFeatureImplications) {
//  NumericFeature A("A", false, std::pair<int, int>(0, 1));
//  NumericFeature B("B", false, std::pair<int, int>(0, 1));
//
//  B.addImplication(&A);
//
//  EXPECT_EQ(std::distance(B.implications_begin(), B.implications_end()), 1);
//  EXPECT_EQ("A", (*B.implications().begin())->getName());
//}
//
// TEST(NumericFeature, NumericFeatureAlternatives) {
//  NumericFeature A("A", false, std::pair<int, int>(0, 1));
//  NumericFeature B("B", false, std::pair<int, int>(0, 1));
//
//  B.addAlternative(&A);
//
//  EXPECT_EQ(std::distance(B.alternatives_begin(), B.alternatives_end()), 1);
//  EXPECT_EQ("A", (*B.alternatives().begin())->getName());
//}
} // namespace vara::feature

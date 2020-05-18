#include "vara/Feature/Feature.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(BinaryFeature, BinaryFeatureBasics) {
  BinaryFeature A("A", true);

  EXPECT_EQ("A", A.getName());
  EXPECT_TRUE(A.isOptional());
  EXPECT_TRUE(A.isRoot());
}

TEST(BinaryFeature, BinaryFeatureRoot) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addParent(&A);

  EXPECT_TRUE(A.isRoot());
  EXPECT_FALSE(B.isRoot());
  EXPECT_EQ(std::distance(A.parents().begin(), A.parents().end()), 0);
  EXPECT_EQ(std::distance(B.parents().begin(), B.parents().end()), 1);
  EXPECT_EQ("A", (*B.parents().begin())->getName());
}

TEST(BinaryFeature, BinaryFeatureChildren) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addChild(&A);

  EXPECT_EQ(std::distance(B.begin(), B.end()), 1);
  EXPECT_EQ("A", (*B.begin())->getName());
  EXPECT_EQ(std::distance(B.children().begin(), B.children().end()), 1);
  EXPECT_EQ("A", (*B.children().begin())->getName());
}

TEST(BinaryFeature, BinaryFeatureExclude) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addExclude(&A);

  EXPECT_EQ(std::distance(B.excludes_begin(), B.excludes_end()), 1);
  EXPECT_EQ("A", (*B.excludes().begin())->getName());
}

TEST(BinaryFeature, BinaryFeatureImplications) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addImplication(&A);

  EXPECT_EQ(std::distance(B.implications_begin(), B.implications_end()), 1);
  EXPECT_EQ("A", (*B.implications().begin())->getName());
}

TEST(BinaryFeature, BinaryFeatureAlternatives) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addAlternative(&A);

  EXPECT_EQ(std::distance(B.alternatives_begin(), B.alternatives_end()), 1);
  EXPECT_EQ("A", (*B.alternatives().begin())->getName());
}
} // namespace vara::feature

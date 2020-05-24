#include "vara/Feature/Feature.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(BinaryFeature, basicAccessors) {
  BinaryFeature A("A", true);

  EXPECT_EQ("A", A.getName());
  EXPECT_TRUE(A.isOptional());
  EXPECT_TRUE(A.isRoot());
}

TEST(BinaryFeature, addParent) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addParent(&A);

  EXPECT_TRUE(A.isRoot());
  EXPECT_FALSE(B.isRoot());
  EXPECT_EQ(std::distance(A.parents().begin(), A.parents().end()), 0);
  EXPECT_EQ(std::distance(B.parents().begin(), B.parents().end()), 1);
  EXPECT_EQ("A", (*B.parents().begin())->getName());
}

TEST(BinaryFeature, isParent) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addParent(&A);

  EXPECT_TRUE(B.isParent(&A));
  EXPECT_FALSE(A.isParent(&B));
}

TEST(BinaryFeature, addChild) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addChild(&A);

  EXPECT_EQ(std::distance(B.begin(), B.end()), 1);
  EXPECT_EQ("A", (*B.begin())->getName());
  EXPECT_EQ(std::distance(B.children().begin(), B.children().end()), 1);
  EXPECT_EQ("A", (*B.children().begin())->getName());
}

TEST(BinaryFeature, isChild) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addChild(&A);

  EXPECT_TRUE(B.isChild(&A));
  EXPECT_FALSE(A.isChild(&B));
}

TEST(BinaryFeature, addExclude) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addExclude(&A);

  EXPECT_EQ(std::distance(B.excludes_begin(), B.excludes_end()), 1);
  EXPECT_EQ("A", (*B.excludes().begin())->getName());
}

TEST(BinaryFeature, isExcluded) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addExclude(&A);

  EXPECT_TRUE(B.isExcluded(&A));
  EXPECT_FALSE(A.isExcluded(&B));
}

TEST(BinaryFeature, addImplication) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addImplication(&A);

  EXPECT_EQ(std::distance(B.implications_begin(), B.implications_end()), 1);
  EXPECT_EQ("A", (*B.implications().begin())->getName());
}

TEST(BinaryFeature, implicates) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addImplication(&A);

  EXPECT_TRUE(B.implies(&A));
  EXPECT_FALSE(A.implies(&B));
}

TEST(BinaryFeature, addAlternative) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addAlternative(&A);

  EXPECT_EQ(std::distance(B.alternatives_begin(), B.alternatives_end()), 1);
  EXPECT_EQ("A", (*B.alternatives().begin())->getName());
}

TEST(BinaryFeature, isAlternative) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);

  B.addAlternative(&A);

  EXPECT_TRUE(B.isAlternative(&A));
  EXPECT_FALSE(A.isAlternative(&B));
}
} // namespace vara::feature

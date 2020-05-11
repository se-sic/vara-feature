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
  for (auto F : B.parents()) {
    EXPECT_EQ("A", F->getName());
  }
}

TEST(BinaryFeature, BinaryFeatureChildren) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);
  B.addChild(&A);
  EXPECT_EQ(std::distance(B.begin(), B.end()), 1);
  for (auto F : B.children()) {
    EXPECT_EQ("A", F->getName());
  }
}

TEST(BinaryFeature, BinaryFeatureExclude) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);
  B.addExclude(&A);
  EXPECT_EQ(std::distance(B.excludes_begin(), B.excludes_end()), 1);
  for (auto F : B.excludes()) {
    EXPECT_EQ("A", F->getName());
  }
}

TEST(BinaryFeature, BinaryFeatureImplications) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);
  B.addImplication(&A);
  EXPECT_EQ(std::distance(B.implications_begin(), B.implications_end()), 1);
  for (auto F : B.implications()) {
    EXPECT_EQ("A", F->getName());
  }
}

TEST(BinaryFeature, BinaryFeatureAlternatives) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);
  B.addAlternative(&A);
  EXPECT_EQ(std::distance(B.alternatives_begin(), B.alternatives_end()), 1);
  for (auto F : B.alternatives()) {
    EXPECT_EQ("A", F->getName());
  }
}
} // namespace vara::feature

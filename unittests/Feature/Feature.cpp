
#include "vara/Feature/Feature.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Feature, compareEqual) {
  BinaryFeature A0("A", false);
  BinaryFeature A1("A", false);
  BinaryFeature A2("a", false);
  BinaryFeature B("B", false);

  EXPECT_TRUE(A0 == A1);
  EXPECT_TRUE(A0 == A2);
  EXPECT_FALSE(A0 == B);
}

TEST(Feature, compareLess) {
  BinaryFeature A("A", false);
  BinaryFeature B("B", false);
  BinaryFeature R("R", false);
  BinaryFeature RLower("r", false);

  EXPECT_TRUE(A < B);
  EXPECT_FALSE(B < A);
  EXPECT_TRUE(B < R);
  EXPECT_TRUE(B < RLower);
  // These features are the same, therefore, not smaller
  EXPECT_FALSE(R < RLower);
}
} // namespace vara::feature

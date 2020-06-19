
#include "vara/Feature/Feature.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Feature, equal) {
  BinaryFeature A0("A");
  BinaryFeature A1("A");
  BinaryFeature A2("a");
  BinaryFeature B("B");

  EXPECT_TRUE(A0 == A1);
  EXPECT_TRUE(A0 == A2);
  EXPECT_FALSE(A0 == B);
}
} // namespace vara::feature

#include "vara/Feature/FeatureSourceRange.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureRevisionRange, full) {
  auto R = FeatureSourceRange::FeatureRevisionRange(
      "94fe792df46e64f438720295742b3b72c407cab6",
      "1ed40f72e772adaa3adfcc94b9f038e4f3382339");

  EXPECT_EQ(R.introducingCommit(), "94fe792df46e64f438720295742b3b72c407cab6");
  ASSERT_TRUE(R.hasRemovingCommit());
  EXPECT_EQ(R.removingCommit(), "1ed40f72e772adaa3adfcc94b9f038e4f3382339");
}

TEST(FeatureRevisionRange, introduced) {
  auto R = FeatureSourceRange::FeatureRevisionRange(
      "94fe792df46e64f438720295742b3b72c407cab6");

  EXPECT_EQ(R.introducingCommit(), "94fe792df46e64f438720295742b3b72c407cab6");
  EXPECT_FALSE(R.hasRemovingCommit());
}

} // namespace vara::feature

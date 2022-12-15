#include "vara/Feature/FeatureSourceRange.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureRevisionRange, full) {
  auto R = FeatureSourceRange::FeatureRevisionRange(
      "94fe792df46e64f438720295742b3b72c407cab6",
      "1ed40f72e772adaa3adfcc94b9f038e4f3382339");

  EXPECT_EQ(R.getIntroducedCommit(),
            "94fe792df46e64f438720295742b3b72c407cab6");
  ASSERT_TRUE(R.hasRemovedCommit());
  EXPECT_EQ(R.getRemovedCommit(), "1ed40f72e772adaa3adfcc94b9f038e4f3382339");
}

TEST(FeatureRevisionRange, introduced) {
  auto R = FeatureSourceRange::FeatureRevisionRange(
      "94fe792df46e64f438720295742b3b72c407cab6");

  EXPECT_EQ(R.getIntroducedCommit(),
            "94fe792df46e64f438720295742b3b72c407cab6");
  EXPECT_FALSE(R.hasRemovedCommit());
}

} // namespace vara::feature

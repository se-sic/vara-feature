#include "vara/Feature/FeatureSourceRange.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(Location, basicAccessors) {
  auto TestLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);

  EXPECT_EQ(TestLCO.getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getColumnOffset(), 4);
}

TEST(Location, comparison) {
  auto SelfLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);
  auto OtherLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);

  EXPECT_EQ(SelfLCO, OtherLCO);
}

TEST(Location, full) {
  auto L = FeatureSourceRange(fs::current_path(),
                              FeatureSourceRange::FeatureSourceLocation(1, 4),
                              FeatureSourceRange::FeatureSourceLocation(3, 5));

  EXPECT_EQ(L.getPath(), fs::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
}
} // namespace vara::feature

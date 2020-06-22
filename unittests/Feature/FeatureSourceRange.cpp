#include "vara/Feature/FeatureSourceRange.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureSourceRange, comparison) {
  auto SelfLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);
  auto OtherLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);

  EXPECT_EQ(SelfLCO, OtherLCO);
}

TEST(FeatureSourceRange, full) {
  auto L = FeatureSourceRange(fs::current_path(),
                              FeatureSourceRange::FeatureSourceLocation(1, 4),
                              FeatureSourceRange::FeatureSourceLocation(3, 5));

  EXPECT_EQ(L.getPath(), fs::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
}

TEST(FeatureSourceLocation, basicAccessors) {
  auto TestLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);

  EXPECT_EQ(TestLCO.getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getColumnOffset(), 4);
}

TEST(FeatureSourceLocation, basicSetter) {
  auto TestLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);
  TestLCO.setLineNumber(4);
  TestLCO.setColumnOffset(5);

  EXPECT_EQ(TestLCO.getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getColumnOffset(), 5);
}

TEST(FeatureSourceLocation, comparison) {
  auto SelfLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);
  auto OtherLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);

  EXPECT_EQ(SelfLCO, OtherLCO);
}

TEST(FeatureSourceRange, basicAccessors) {
  auto L = FeatureSourceRange(fs::current_path(),
                              FeatureSourceRange::FeatureSourceLocation(1, 4),
                              FeatureSourceRange::FeatureSourceLocation(3, 5));

  EXPECT_EQ(L.getPath(), fs::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
}

TEST(FeatureSourceRange, onlyStart) {
  auto L = FeatureSourceRange(fs::current_path(),
                              FeatureSourceRange::FeatureSourceLocation(1, 4));

  EXPECT_TRUE(L.hasStart());
  EXPECT_FALSE(L.hasEnd());
}

TEST(FeatureSourceRange, onlyEnd) {
  auto L = FeatureSourceRange(fs::current_path(), std::nullopt,
                              FeatureSourceRange::FeatureSourceLocation(3, 5));

  EXPECT_FALSE(L.hasStart());
  EXPECT_TRUE(L.hasEnd());
}
} // namespace vara::feature

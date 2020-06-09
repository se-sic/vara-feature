#include "vara/Feature/Location.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(LineColumnOffset, basicAccessors) {
  auto TestLCO = Location::LineColumnOffset(3, 4);

  EXPECT_EQ(TestLCO.getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getColumnOffset(), 4);
}

TEST(LineColumnOffset, basicSetter) {
  auto TestLCO = Location::LineColumnOffset(3, 4);
  TestLCO.setLineNumber(4);
  TestLCO.setColumnOffset(5);

  EXPECT_EQ(TestLCO.getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getColumnOffset(), 5);
}

TEST(LineColumnOffset, comparison) {
  auto SelfLCO = Location::LineColumnOffset(3, 4);
  auto OtherLCO = Location::LineColumnOffset(3, 4);

  EXPECT_EQ(SelfLCO, OtherLCO);
}

TEST(Location, basicAccessors) {
  auto L = Location(fs::current_path(), Location::LineColumnOffset(1, 4),
                    Location::LineColumnOffset(3, 5));

  EXPECT_EQ(L.getPath(), fs::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
}
} // namespace vara::feature

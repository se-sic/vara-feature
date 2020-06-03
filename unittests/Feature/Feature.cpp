
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

TEST(Feature, locationAccessors) {
  Location::LineColumnOffset Start(3,4);
  Location::LineColumnOffset End(3,20);
  BinaryFeature TestLCO("TEST", false, Location("path", Start, End));

  EXPECT_EQ(TestLCO.getLocation()->getStart()->getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getLocation()->getStart()->getColumnOffset(), 4);
  EXPECT_EQ(TestLCO.getLocation()->getEnd()->getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getLocation()->getEnd()->getColumnOffset(), 20);
}

TEST(Feature, locationSetters) {
  Location::LineColumnOffset Start(3,4);
  Location::LineColumnOffset End(3,20);
  BinaryFeature TestLCO("TEST", false, Location("path", Start, End));

  TestLCO.getLocation()->getStart()->setLineNumber(4);
  TestLCO.getLocation()->getStart()->setColumnOffset(2);
  TestLCO.getLocation()->getEnd()->setLineNumber(4);
  TestLCO.getLocation()->getEnd()->setColumnOffset(18);

  EXPECT_EQ(TestLCO.getLocation()->getStart()->getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getLocation()->getStart()->getColumnOffset(), 2);
  EXPECT_EQ(TestLCO.getLocation()->getEnd()->getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getLocation()->getEnd()->getColumnOffset(), 18);
}

} // namespace vara::feature

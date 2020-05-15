#include "vara/Feature/Location.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Location, LocationBasics) {
  auto L = Location(std::filesystem::current_path(),
                    Location::LineColumnOffset(1, 4),
                    Location::LineColumnOffset(3, 5));
  EXPECT_EQ(L.getPath(), std::filesystem::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
}
} // namespace vara::feature

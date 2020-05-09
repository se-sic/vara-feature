#include "vara/Feature/Feature.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(BinaryFeature, getName) {
  BinaryFeature F("Test", false, std::nullopt);
  EXPECT_EQ("Test", F.getName());
}
} // namespace vara::feature

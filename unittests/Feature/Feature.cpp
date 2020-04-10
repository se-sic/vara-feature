#include "vara/Feature/Feature.h"

#include "gtest/gtest.h"

namespace {
TEST(Feature, getSize) {
  vara::feature::Feature F;
  F.addStuff(42);
  EXPECT_EQ(1U, F.getSize());
}
} // namespace

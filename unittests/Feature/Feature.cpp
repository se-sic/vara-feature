#include "vara/Feature/Feature.h"

#include "gtest/gtest.h"

namespace {
TEST(Feature, getSize) {
  vara::feature::Feature f;
  f.addStuff(42);
  EXPECT_EQ(1u, f.getSize());
}
}

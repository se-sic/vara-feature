#include "vara/Sampling/SamplingMethods.h"

#include "gtest/gtest.h"

namespace vara::sampling {
TEST(BasicSampling, testIfLibrarySetupWorks) {
  auto [Dens1, Prob1, Quant1] = getExampleValues();

  EXPECT_DOUBLE_EQ(Dens1, 0.25);
  EXPECT_DOUBLE_EQ(Prob1, 0.5);
  EXPECT_NEAR(Quant1, -2.218875, 0.001);
}
} // namespace vara::sampling

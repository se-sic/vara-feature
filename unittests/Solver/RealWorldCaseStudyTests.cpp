#include "vara/Feature/FeatureModel.h"
#include "vara/Solver/ConfigurationFactory.h"

#include "Utils/UnittestHelper.h"

#include "llvm/ADT/StringRef.h"
#include "gtest/gtest.h"

namespace vara::solver {

TEST(SolverAPI, TestDuneConfigurations) {
  auto FMBin = feature::loadFeatureModel(getTestResource("test_dune_bin.xml"));
  auto ConfigResultBin = ConfigurationFactory::getAllConfigs(*FMBin);
  EXPECT_TRUE(ConfigResultBin);
  EXPECT_EQ(ConfigResultBin.extractValue().size(), 2304);

  // Perform a test on Dune with numeric features without step functions
  auto FMNumExp =
      feature::loadFeatureModel(getTestResource("test_dune_num_explicit.xml"));
  auto ConfigResultNumExp = ConfigurationFactory::getAllConfigs(*FMNumExp);
  EXPECT_TRUE(ConfigResultNumExp);
  EXPECT_EQ(ConfigResultNumExp.extractValue().size(), 2304);

  // Perform a test on Dune with numeric features with step functions
  auto FMNumStep =
      feature::loadFeatureModel(getTestResource("test_dune_num_explicit.xml"));
  auto ConfigResultNumStep = ConfigurationFactory::getAllConfigs(*FMNumStep);
  EXPECT_TRUE(ConfigResultNumStep);
  EXPECT_EQ(ConfigResultNumStep.extractValue().size(), 2304);
}

TEST(SolverAPI, SmallTestMixedConstraints) {
  auto FMNumStep =
      feature::loadFeatureModel(getTestResource("test_mixed_constraints.xml"));
  auto ConfigResultNumStep = ConfigurationFactory::getAllConfigs(*FMNumStep);
  EXPECT_TRUE(ConfigResultNumStep);
  EXPECT_EQ(ConfigResultNumStep.extractValue().size(), 4);
}

TEST(SolverAPI, TestMixedConstraints) {
  auto FMNumStep =
      feature::loadFeatureModel(getTestResource("test_hsqldb_num.xml"));
  auto ConfigResultNumStep = ConfigurationFactory::getAllConfigs(*FMNumStep);
  EXPECT_TRUE(ConfigResultNumStep);
  EXPECT_EQ(ConfigResultNumStep.extractValue().size(), 864);
}

} // namespace vara::solver

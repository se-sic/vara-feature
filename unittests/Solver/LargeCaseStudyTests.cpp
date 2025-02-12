#include "vara/Feature/FeatureModel.h"
#include "vara/Solver/ConfigurationFactory.h"

#include "Utils/UnittestHelper.h"

#include "llvm/ADT/StringRef.h"
#include "gtest/gtest.h"

namespace vara::solver {

TEST(SolverAPI, TestHipaccConfigurations) {
  //  Test the Hipacc feature model, which has (1) numeric features and
  //  (2) many cross-tree-constraints.
  //  To test also alternative groups, we test Hipacc with numeric features,
  //  but also with its discretized counterpart (i.e., every numeric feature
  //  is converted into multiple binary features)
  auto FmNum =
      feature::loadFeatureModel(getTestResource("test_hipacc_num.xml"));
  EXPECT_TRUE(FmNum);
  auto ConfigResult = ConfigurationFactory::getAllConfigs(*FmNum);
  EXPECT_TRUE(ConfigResult);
  EXPECT_EQ(ConfigResult.extractValue().size(), 13485);
  auto FMBin =
      feature::loadFeatureModel(getTestResource("test_hipacc_bin.xml"));
  auto ConfigResultBin = ConfigurationFactory::getAllConfigs(*FMBin);
  EXPECT_TRUE(ConfigResultBin);
  EXPECT_EQ(ConfigResultBin.extractValue().size(), 13485);
}

} // namespace vara::solver

#include "vara/Feature/FeatureModelBuilder.h"
#include "vara/Feature/FeatureModelParser.h"
#include "vara/Solver/ConfigurationFactory.h"

#include "Utils/UnittestHelper.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

namespace vara::solver {

std::unique_ptr<vara::feature::FeatureModel>
buildFeatureModel(llvm::StringRef Path) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(Path);
  // auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource(Path));
  assert(FS);
  auto P = vara::feature::FeatureModelXmlParser(FS.get()->getBuffer().str());
  assert(P.verifyFeatureModel());
  return P.buildFeatureModel();
}

TEST(SolverAPI, TestDuneConfigurations) {
  auto FMBin =
      buildFeatureModel("../../../unittests/resources/xml/test_dune_bin.xml");
  // auto FMBin = buildFeatureModel("test_dune_bin.xml");
  auto ConfigResultBin = ConfigurationFactory::getAllConfigs(*FMBin);
  EXPECT_TRUE(ConfigResultBin);
  EXPECT_EQ(ConfigResultBin.extractValue()->size(), 2304);

  // Perform a test on Dune with numeric features without step functions
  auto FMNumExp = buildFeatureModel(
      "../../../unittests/resources/xml/test_dune_num_explicit.xml");
  auto ConfigResultNumExp = ConfigurationFactory::getAllConfigs(*FMNumExp);
  EXPECT_TRUE(ConfigResultNumExp);
  EXPECT_EQ(ConfigResultNumExp.extractValue()->size(), 2304);

  // Perform a test on Dune with numeric features with step functions
  auto FMNumStep = buildFeatureModel(
      "../../../unittests/resources/xml/test_dune_num_explicit.xml");
  auto ConfigResultNumStep = ConfigurationFactory::getAllConfigs(*FMNumStep);
  EXPECT_TRUE(ConfigResultNumStep);
  EXPECT_EQ(ConfigResultNumStep.extractValue()->size(), 2304);
}

TEST(SolverAPI, TestHipaccConfigurations) {
  GTEST_SKIP();
  //  Test the Hipacc feature model, which has (1) numeric features and
  //  (2) many cross-tree-constraints.
  //  To test also alternative groups, we test Hipacc with numeric features,
  //  but also with its discretized counterpart (i.e., every numeric feature
  //  is converted into multiple binary features)
  auto FmNum =
      buildFeatureModel("../../../unittests/resources/xml/test_hipacc_num.xml");
  EXPECT_TRUE(FmNum);
  auto ConfigResult = ConfigurationFactory::getAllConfigs(*FmNum);
  EXPECT_TRUE(ConfigResult);
  EXPECT_EQ(ConfigResult.extractValue()->size(), 13485);
  auto FMBin =
      buildFeatureModel("../../../unittests/resources/xml/test_hipacc_bin.xml");
  auto ConfigResultBin = ConfigurationFactory::getAllConfigs(*FMBin);
  EXPECT_TRUE(ConfigResultBin);
  EXPECT_EQ(ConfigResultBin.extractValue()->size(), 13485);
}

} // namespace vara::solver
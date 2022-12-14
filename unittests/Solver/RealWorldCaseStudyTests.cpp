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
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource(Path));
  assert(FS);
  auto P = vara::feature::FeatureModelXmlParser(FS.get()->getBuffer().str());
  assert(P.verifyFeatureModel());
  return P.buildFeatureModel();
}

TEST(SolverAPI, TestHipaccConfigurations) {
  // Test the Hipacc feature model, which has (1) numeric features and
  // (2) many cross-tree-constraints.
  // To test also alternative groups, we test Hipacc with numeric features,
  // but also with its discretized counterpart (i.e., every numeric feature
  // is converted into multiple binary features)
  auto FmNum = buildFeatureModel("test_hipacc_num.xml");
  EXPECT_TRUE(FmNum);
  auto ConfigResult = ConfigurationFactory::getAllConfigs(*FmNum);
  EXPECT_TRUE(ConfigResult);
  EXPECT_EQ(ConfigResult.extractValue()->size(), 13485);
  auto FMBin = buildFeatureModel("test_hipacc_bin.xml");
  auto ConfigResultBin = ConfigurationFactory::getAllConfigs(*FmNum);
  EXPECT_TRUE(ConfigResultBin);
  EXPECT_EQ(ConfigResult.extractValue()->size(),
            ConfigResultBin.extractValue()->size());
}

} // namespace vara::solver
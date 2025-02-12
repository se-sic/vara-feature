#include "vara/Feature/FeatureModelParser.h"
#include "vara/Sampling/SampleSetParser.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

inline std::string getTestResource(llvm::StringRef ResourcePath = "") {
  constexpr const char *BasePath = "resources/";
  return (llvm::Twine(BasePath) + ResourcePath).str();
}

inline void testConfigurationOption(
    const std::unique_ptr<vara::feature::Configuration> &Config,
    llvm::StringRef Feature, llvm::StringRef ExpectedValue) {
  auto Value = Config->configurationOptionValue(Feature);
  EXPECT_TRUE(Value);
  EXPECT_EQ(Value.value(), ExpectedValue.str());
}

namespace vara::sampling {
TEST(SampleSetParser, testCSVParsing) {
  auto CsvPath = getTestResource("configs_dune.csv");

  auto FMFS =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("test_dune_num.xml"));
  EXPECT_TRUE(FMFS);
  auto P = vara::feature::FeatureModelXmlParser(FMFS.get()->getBuffer().str());
  auto FM = P.buildFeatureModel();
  auto Configs = SampleSetParser::readConfigurations(*FM, CsvPath);
  ASSERT_EQ(Configs.size(), 40);

  // Check the configuration options of the first configuration
  const auto &FirstConfig = Configs.back();
  testConfigurationOption(FirstConfig, "root", "true");
  testConfigurationOption(FirstConfig, "Precon", "true");
  testConfigurationOption(FirstConfig, "Solver", "true");
  testConfigurationOption(FirstConfig, "SeqGS", "true");
  testConfigurationOption(FirstConfig, "SeqSOR", "false");
  testConfigurationOption(FirstConfig, "CGSolver", "false");
  testConfigurationOption(FirstConfig, "BiCGSTABSolver", "false");
  testConfigurationOption(FirstConfig, "LoopSolver", "false");
  testConfigurationOption(FirstConfig, "GradientSolver", "true");
  testConfigurationOption(FirstConfig, "pre", "0");
  testConfigurationOption(FirstConfig, "post", "3");
  testConfigurationOption(FirstConfig, "cells", "55");

  // Check the configuration options of the last configuration
  const auto &LastConfig = Configs.front();
  testConfigurationOption(LastConfig, "root", "true");
  testConfigurationOption(LastConfig, "Precon", "true");
  testConfigurationOption(LastConfig, "Solver", "true");
  testConfigurationOption(LastConfig, "SeqGS", "false");
  testConfigurationOption(LastConfig, "SeqSOR", "true");
  testConfigurationOption(LastConfig, "CGSolver", "false");
  testConfigurationOption(LastConfig, "BiCGSTABSolver", "false");
  testConfigurationOption(LastConfig, "LoopSolver", "true");
  testConfigurationOption(LastConfig, "GradientSolver", "false");
  testConfigurationOption(LastConfig, "pre", "3");
  testConfigurationOption(LastConfig, "post", "0");
  testConfigurationOption(LastConfig, "cells", "52");
}
} // namespace vara::sampling

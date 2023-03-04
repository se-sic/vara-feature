#include "vara/Feature/FeatureModelParser.h"
#include "vara/Sampling/SampleSetParser.h"
#include "vara/Sampling/SampleSetWriter.h"

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
TEST(SampleSetWriter, testWriting) {
  auto CsvPath = getTestResource("configs_dune.csv");

  auto FMFS =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("test_dune_num.xml"));
  EXPECT_TRUE(FMFS);
  auto P = vara::feature::FeatureModelXmlParser(FMFS.get()->getBuffer().str());
  auto FM = P.buildFeatureModel();
  auto Configs = SampleSetParser::readConfigurations(*FM, CsvPath);
  ASSERT_EQ(Configs->size(), 40);

  auto ActualString = SampleSetWriter::writeConfigurations(*FM, *Configs);
  auto ExpectedString =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("dune_configs.yml"))
          .get()
          ->getBuffer()
          .str();
  ASSERT_EQ(ExpectedString, ActualString);
}
} // namespace vara::sampling

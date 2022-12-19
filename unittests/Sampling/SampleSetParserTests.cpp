#include "vara/Feature/FeatureModelParser.h"
#include "vara/Sampling/SampleSetParser.h"
#include "Feature/UnittestHelper.h"

#include "gtest/gtest.h"
#include "llvm/Support/MemoryBuffer.h"

namespace vara::sampling {
TEST(SampleSetParser, testCSVParsing) {
  const auto *FS = "../../../unittests/resources/csv/configs_dune.csv";
  // auto FS = getTestResource("configs_dune.csv");

  auto FMFS = llvm::MemoryBuffer::getFileAsStream(
      "../../../unittests/resources/xml/test_dune_num_explicit.xml");
  //  auto FMFS = llvm::MemoryBuffer::getFileAsStream(
  //    getTestResource("test_dune_num_explicit.xml"));
  auto P = vara::feature::FeatureModelXmlParser(FMFS.get()->getBuffer().str());
  auto FM = P.buildFeatureModel();
  auto Configs = SampleSetParser::readConfigurations(*FM, FS);
  EXPECT_EQ(Configs->size(), 40);
}
} // namespace vara::sampling

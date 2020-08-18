#include "vara/Feature/FeatureModelWriter.h"
#include "vara/Feature/FeatureModelParser.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

#include <iostream>

namespace vara::feature {

const std::string TESTFILEDIRPATH = "../../../test/";

TEST(XmlWriter, testtest) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(TESTFILEDIRPATH + "test.xml");
  assert(FS && "Input file could not be read");
  auto FM = FeatureModelXmlParser(FS.get()->getBuffer()).buildFeatureModel();

  FeatureModelXmlWriter Fmxw = FeatureModelXmlWriter(*FM);
  auto Output = Fmxw.writeFeatureModel();
  assert(Output && "Feature model could not be written");

  std::string ActualOutput = Output.value();
  FS = llvm::MemoryBuffer::getFileAsStream(TESTFILEDIRPATH + "test_out.xml");
  assert(FS && "Comparisson file could not be read");
  std::string ExpectedOutput = FS.get()->getBuffer();
  EXPECT_EQ(ActualOutput, ExpectedOutput);
}

} // namespace vara::feature

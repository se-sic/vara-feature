#include "vara/Feature/FeatureModelWriter.h"
#include "vara/Feature/FeatureModelParser.h"

#include "UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

#include <iostream>

namespace vara::feature {

TEST(XmlWriter, children) {
  auto FS =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("test_children.xml"));
  EXPECT_TRUE(FS && "Input file could not be read");
  auto FM = FeatureModelXmlParser(FS.get()->getBuffer()).buildFeatureModel();

  FeatureModelXmlWriter Fmxw = FeatureModelXmlWriter(*FM);
  auto Output = Fmxw.writeFeatureModel();
  EXPECT_TRUE(Output.has_value());
  std::string ActualOutput = Output.value();
  EXPECT_FALSE(ActualOutput.empty());

  std::string ExpectedOutput = FS.get()->getBuffer();
  EXPECT_EQ(ActualOutput, ExpectedOutput);
}

TEST(XmlWriter, excludes) {
  auto FS =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("test_excludes.xml"));
  EXPECT_TRUE(FS && "Input file could not be read");
  auto FM = FeatureModelXmlParser(FS.get()->getBuffer()).buildFeatureModel();

  FeatureModelXmlWriter Fmxw = FeatureModelXmlWriter(*FM);
  auto Output = Fmxw.writeFeatureModel();
  EXPECT_TRUE(Output.has_value());
  std::string ActualOutput = Output.value();
  EXPECT_FALSE(ActualOutput.empty());

  std::string ExpectedOutput = FS.get()->getBuffer();
  EXPECT_EQ(ActualOutput, ExpectedOutput);
}

TEST(XmlWriter, test) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("test.xml"));
  EXPECT_TRUE(FS && "Input file could not be read");
  auto FM = FeatureModelXmlParser(FS.get()->getBuffer()).buildFeatureModel();

  FeatureModelXmlWriter Fmxw = FeatureModelXmlWriter(*FM);
  auto Output = Fmxw.writeFeatureModel();
  EXPECT_TRUE(Output.has_value());
  std::string ActualOutput = Output.value();
  EXPECT_FALSE(ActualOutput.empty());

  FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("test_out.xml"));
  EXPECT_TRUE(FS && "Comparisson file could not be read");
  std::string ExpectedOutput = FS.get()->getBuffer();
  EXPECT_EQ(ActualOutput, ExpectedOutput);
}

} // namespace vara::feature

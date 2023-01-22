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
  auto FM =
      FeatureModelXmlParser(FS.get()->getBuffer().str()).buildFeatureModel();

  FeatureModelXmlWriter FMX = FeatureModelXmlWriter(*FM);
  auto Output = FMX.writeFeatureModel();
  EXPECT_TRUE(Output.hasValue());
  std::string ActualOutput = Output.getValue();
  EXPECT_FALSE(ActualOutput.empty());

  std::string ExpectedOutput = FS.get()->getBuffer().str();
  EXPECT_EQ(ExpectedOutput, ActualOutput);
}

TEST(XmlWriter, constraints) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_constraints.xml"));
  EXPECT_TRUE(FS && "Input file could not be read");
  auto FM =
      FeatureModelXmlParser(FS.get()->getBuffer().str()).buildFeatureModel();

  FeatureModelXmlWriter FMX = FeatureModelXmlWriter(*FM);
  auto Output = FMX.writeFeatureModel();

  EXPECT_TRUE(Output.hasValue());
  std::string ActualOutput = Output.getValue();
  EXPECT_FALSE(ActualOutput.empty());

  std::string ExpectedOutput = FS.get()->getBuffer().str();
  EXPECT_EQ(ExpectedOutput, ActualOutput);
}

TEST(XmlWriter, excludes) {
  auto FS =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("test_excludes.xml"));
  EXPECT_TRUE(FS && "Input file could not be read");
  auto FM =
      FeatureModelXmlParser(FS.get()->getBuffer().str()).buildFeatureModel();

  FeatureModelXmlWriter FMX = FeatureModelXmlWriter(*FM);
  auto Output = FMX.writeFeatureModel();
  EXPECT_TRUE(Output.hasValue());
  std::string ActualOutput = Output.getValue();
  EXPECT_FALSE(ActualOutput.empty());

  std::string ExpectedOutput = FS.get()->getBuffer().str();
  EXPECT_EQ(ExpectedOutput, ActualOutput);
}

TEST(XmlWriter, test) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("test.xml"));
  EXPECT_TRUE(FS && "Input file could not be read");
  auto FM =
      FeatureModelXmlParser(FS.get()->getBuffer().str()).buildFeatureModel();

  FeatureModelXmlWriter FMX = FeatureModelXmlWriter(*FM);
  auto Output = FMX.writeFeatureModel();
  EXPECT_TRUE(Output.hasValue());
  std::string ActualOutput = Output.getValue();
  EXPECT_FALSE(ActualOutput.empty());

  FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("test.xml"));
  EXPECT_TRUE(FS && "Comparison file could not be read");
  std::string ExpectedOutput = FS.get()->getBuffer().str();
  EXPECT_EQ(ExpectedOutput, ActualOutput);
}

} // namespace vara::feature

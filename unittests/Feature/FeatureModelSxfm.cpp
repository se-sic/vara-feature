#include "vara/Feature/FeatureModelParser.h"

#include "UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

#include <iostream>

namespace vara::feature {

TEST(SxfmParser, validation) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("sxfm_example.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM = FeatureModelSxfmParser(FS.get()->getBuffer().str()).buildFeatureModel();
  EXPECT_TRUE(FM);
  EXPECT_EQ(FM->size(), 17);
  EXPECT_EQ(FM->getName(), "My feature model");
}

TEST(SxfmParser, parsing) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("test.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM = FeatureModelSxfmParser(FS.get()->getBuffer().str()).buildFeatureModel();
  EXPECT_TRUE(FM);
  EXPECT_EQ(FM->size(), 37);
  EXPECT_EQ(FM->getName(), "apache");
}

TEST(SxfmParser, wrong_indentation) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("test_wrong_indentation.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM = FeatureModelSxfmParser(FS.get()->getBuffer().str()).buildFeatureModel();
  EXPECT_FALSE(FM);
}

TEST(SxfmParser, wrong_xml_format) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("test_wrong_xml_format.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM = FeatureModelSxfmParser(FS.get()->getBuffer().str());
  EXPECT_FALSE(FM.verifyFeatureModel());
}

} // namespace vara::feature
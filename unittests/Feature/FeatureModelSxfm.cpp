#include "vara/Feature/FeatureModelWriter.h"
#include "vara/Feature/FeatureModelParser.h"

#include "UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

#include <iostream>

namespace vara::feature {

TEST(SxfmParser, validation) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("sxfm_example.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM = FeatureModelSxfmParser(FS.get()->getBuffer()).buildFeatureModel();
  EXPECT_TRUE(FM);
}

TEST(SxfmParser, parsing) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("test.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM = FeatureModelSxfmParser(FS.get()->getBuffer()).buildFeatureModel();
  EXPECT_TRUE(FM);
}

} // namespace vara::feature
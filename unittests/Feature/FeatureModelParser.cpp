#include "vara/Feature/FeatureModelParser.h"

#include "UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModelParser, onlyChildren) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_only_children.xml"));
  assert(FS);

  auto FM = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(FM.verifyFeatureModel());
  EXPECT_TRUE(FM.buildFeatureModel());
}

TEST(FeatureModelParser, onlyParents) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_only_parents.xml"));
  assert(FS);

  auto FM = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(FM.verifyFeatureModel());
  EXPECT_TRUE(FM.buildFeatureModel());
}

TEST(FeatureModelParser, outOfOrder) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_out_of_order.xml"));
  assert(FS);

  auto FM = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(FM.verifyFeatureModel());
  EXPECT_TRUE(FM.buildFeatureModel());
}

} // namespace vara::feature
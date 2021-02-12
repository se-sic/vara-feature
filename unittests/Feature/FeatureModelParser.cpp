#include "vara/Feature/FeatureModelParser.h"

#include "UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModelParser, onlyChildren) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_only_children.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, onlyParents) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_only_parents.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, outOfOrder) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_out_of_order.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, missingRoot) {
  // this test is expected to fail
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_missing_root.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, tagNotClosed) {
  // this test is expected to fail
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_tags_not_closed.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, nameTagNotProvided) {
  // this test is expected to fail
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_name_tag_not_provided.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, circularParentship) {
  // this test is expected to fail
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_circular_parentship.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, excludeMissingFeature) {
  // this test is expected to fail
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_exclude_missing_feature.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, wrongOptionalFlag) {
  // this test is expected to fail
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_wrong_optional_flag.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, emptyExclusions) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_empty_exclusions.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, specialCharInName) {
  // this test is expected to fail
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_special_char_in_name.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, randomTextInTag) {
  // this test is expected to fail
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_random_text_in_tag.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, docTagNotProvided) {
  // is this test expected to fail or pass?
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_tag_not_provided.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}
} // namespace vara::feature

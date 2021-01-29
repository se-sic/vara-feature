#include "vara/Feature/FeatureModelParser.h"

#include "UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

#include <iostream>

namespace vara::feature {

TEST(SxfmParser, validation) {
  auto FS =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("sxfm_example.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM =
      FeatureModelSxfmParser(FS.get()->getBuffer().str()).buildFeatureModel();
  EXPECT_TRUE(FM);
  EXPECT_EQ(FM->size(), 17);
  EXPECT_EQ(FM->getName(), "My feature model");

  // Check features
  EXPECT_TRUE(FM->getFeature("man1"));
  EXPECT_TRUE(FM->getFeature("group_1"));

  // Check feature types
  EXPECT_FALSE(FM->getFeature("man1")->isOptional());
  EXPECT_EQ(FM->getFeature("group_1")->getKind(),
            Feature::FeatureKind::FK_BINARY);
}

TEST(SxfmParser, parsing) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource("test.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM =
      FeatureModelSxfmParser(FS.get()->getBuffer().str()).buildFeatureModel();
  EXPECT_TRUE(FM);
  EXPECT_EQ(FM->size(), 37);
  EXPECT_EQ(FM->getName(), "apache");

  // Check features
  EXPECT_TRUE(FM->getFeature("root"));
  EXPECT_TRUE(FM->getFeature("threadCount"));
  EXPECT_TRUE(FM->getFeature("threadCount_64"));
  EXPECT_TRUE(FM->getFeature("Group_0"));
  EXPECT_TRUE(FM->getFeature("tlsMoreBits"));
  EXPECT_TRUE(FM->getFeature("ecdsaCertificate"));
  EXPECT_TRUE(FM->getFeature("tls"));
  EXPECT_TRUE(FM->getFeature("compressionLevel"));

  // Check feature types
  EXPECT_TRUE(FM->getFeature("tls")->isOptional());
  EXPECT_FALSE(FM->getFeature("compressionLevel")->isOptional());
  EXPECT_EQ(FM->getFeature("compressionLevel")->getKind(),
            Feature::FeatureKind::FK_BINARY);
}

TEST(SxfmParser, wrong_indentation) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_wrong_indentation.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM =
      FeatureModelSxfmParser(FS.get()->getBuffer().str()).buildFeatureModel();
  EXPECT_FALSE(FM);
}

TEST(SxfmParser, wrong_xml_format) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_wrong_xml_format.sxfm"));
  EXPECT_TRUE(FS && "Input file could not be read.");
  auto FM = FeatureModelSxfmParser(FS.get()->getBuffer().str());
  EXPECT_FALSE(FM.verifyFeatureModel());
}

} // namespace vara::feature
#include "vara/Feature/FeatureModelParser.h"

#include "UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModelParser, mismatchParentChild) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_mismatch_parent_child.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_FALSE(P.buildFeatureModel());
}

TEST(FeatureModelParser, nonExistentChild) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_non_existent_child.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_FALSE(P.buildFeatureModel());
}

TEST(FeatureModelParser, nonExistentExclude) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_non_existent_exclude.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_FALSE(P.buildFeatureModel());
}

TEST(FeatureModelParser, nonExistentImplication) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_non_existent_implication.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_FALSE(P.buildFeatureModel());
}

TEST(FeatureModelParser, nonExistentParent) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_non_existent_parent.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_FALSE(P.buildFeatureModel());
}

TEST(FeatureModelParser, onlyChildren) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_only_children.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, onlyParents) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_only_parents.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, outOfOrder) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_out_of_order.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

TEST(FeatureModelParser, rootEmptyParent) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_root_empty_parent.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  auto FM = P.buildFeatureModel();
  EXPECT_TRUE(FM);
  EXPECT_TRUE(FM->getRoot());
}

TEST(FeatureModelParser, rootNoParent) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_root_no_parent.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  auto FM = P.buildFeatureModel();
  EXPECT_TRUE(FM);
  EXPECT_TRUE(FM->getRoot());
}

TEST(FeatureModelParser, rootParentRoot) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("xml/test_root_parent_root.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  auto FM = P.buildFeatureModel();
  EXPECT_TRUE(FM);
  EXPECT_TRUE(FM->getRoot());
}

//===----------------------------------------------------------------------===//
//                        XMLAlternatives
//===----------------------------------------------------------------------===//

TEST(FeatureModelParser, detectXMLAlternativesSimple) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("aa", false)->addEdge("a", "aa");
  B.makeFeature<BinaryFeature>("ab", false)->addEdge("a", "ab");

  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("aa", "ab"));
  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("ab", "aa"));
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  ASSERT_TRUE(FeatureModelXmlParser::detectXMLAlternatives(*FM));

  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  if (auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
      R) {
    EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_ALTERNATIVE);
    EXPECT_TRUE(R->hasEdgeFrom(*FM->getFeature("a")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("aa")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
  } else {
    FAIL();
  }
}

TEST(FeatureModelParser, detectXMLAlternativesBroken) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addEdge("a", "aa");
  B.addEdge("a", "ab");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ab", false);

  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("aa", "ab"));
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  ASSERT_TRUE(FeatureModelXmlParser::detectXMLAlternatives(*FM));

  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
}

TEST(FeatureModelParser, detectXMLAlternativesOptionalBroken) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addEdge("a", "aa");
  B.addEdge("a", "ab");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ab", true);

  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("aa", "ab"));
  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("ab", "aa"));
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  ASSERT_TRUE(FeatureModelXmlParser::detectXMLAlternatives(*FM));

  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
}

TEST(FeatureModelParser, detectXMLAlternativesOutOfOrder) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addEdge("a", "ab");
  B.addEdge("a", "aa");
  B.addEdge("a", "ac");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ac", false);
  B.makeFeature<BinaryFeature>("ab", false);

  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("aa", "ab"));
  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("ab", "aa"));
  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("ac", "aa"));
  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("ab", "ac"));
  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("aa", "ac"));
  B.addConstraint(createBinaryConstraint<ExcludesConstraint>("ac", "ab"));
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  ASSERT_TRUE(FeatureModelXmlParser::detectXMLAlternatives(*FM));

  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ac")));
  if (auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
      R) {
    EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_ALTERNATIVE);
    EXPECT_TRUE(R->hasEdgeFrom(*FM->getFeature("a")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("aa")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ac")));
  } else {
    FAIL();
  }
}

} // namespace vara::feature

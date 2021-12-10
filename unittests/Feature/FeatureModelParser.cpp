#include "vara/Feature/FeatureModelParser.h"

#include "UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModelParser, trim) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(
      getTestResource("test_with_whitespaces.xml"));
  assert(FS);

  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());

  EXPECT_TRUE(P.verifyFeatureModel());
  EXPECT_TRUE(P.buildFeatureModel());
}

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

TEST(FeatureModelParser, longRange) {
  auto FS =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("test_numbers.xml"));
  assert(FS);
  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());
  ASSERT_TRUE(P.verifyFeatureModel());
  auto FM = P.buildFeatureModel();
  ASSERT_TRUE(FM);

  auto *F = llvm::dyn_cast_or_null<NumericFeature>(FM->getFeature("A"));
  ASSERT_TRUE(F);

  EXPECT_TRUE(
      (std::holds_alternative<NumericFeature::ValueRangeType>(F->getValues())));
  EXPECT_EQ((std::get<NumericFeature::ValueRangeType>(F->getValues())).first,
            -9223372036854775808L);
  EXPECT_EQ((std::get<NumericFeature::ValueRangeType>(F->getValues())).second,
            9223372036854775807L);
}

TEST(FeatureModelParser, longList) {
  auto FS =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("test_numbers.xml"));
  assert(FS);
  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());
  ASSERT_TRUE(P.verifyFeatureModel());
  auto FM = P.buildFeatureModel();
  ASSERT_TRUE(FM);

  auto *F = llvm::dyn_cast_or_null<NumericFeature>(FM->getFeature("B"));
  ASSERT_TRUE(F);

  EXPECT_TRUE(
      std::holds_alternative<NumericFeature::ValueListType>(F->getValues()));
  EXPECT_THAT(
      std::get<NumericFeature::ValueListType>(F->getValues()),
      testing::ElementsAre(-9223372036854775808L, 0L, 9223372036854775807L));
}

TEST(FeatureModelParser, scientific) {
  auto FS =
      llvm::MemoryBuffer::getFileAsStream(getTestResource("test_numbers.xml"));
  assert(FS);
  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());
  ASSERT_TRUE(P.verifyFeatureModel());
  auto FM = P.buildFeatureModel();
  ASSERT_TRUE(FM);

  auto *F = llvm::dyn_cast_or_null<NumericFeature>(FM->getFeature("C"));
  ASSERT_TRUE(F);

  EXPECT_TRUE(
      (std::holds_alternative<NumericFeature::ValueRangeType>(F->getValues())));
  EXPECT_EQ((std::get<NumericFeature::ValueRangeType>(F->getValues())).first,
            0);
  EXPECT_EQ((std::get<NumericFeature::ValueRangeType>(F->getValues())).second,
            4000);
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

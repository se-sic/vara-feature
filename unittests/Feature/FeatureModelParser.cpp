#include "vara/Feature/FeatureModelParser.h"

#include "Utils/UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace vara::feature {

std::unique_ptr<FeatureModel> buildFeatureModel(llvm::StringRef Path) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource(Path));
  assert(FS);
  auto P = FeatureModelXmlParser(FS.get()->getBuffer().str());
  assert(P.verifyFeatureModel());
  return P.buildFeatureModel();
}

TEST(FeatureModelParser, trim) {
  EXPECT_TRUE(buildFeatureModel("test_with_whitespaces.xml"));
}

TEST(FeatureModelParser, onlyChildren) {
  EXPECT_TRUE(buildFeatureModel("test_only_children.xml"));
}

TEST(FeatureModelParser, onlyParents) {
  EXPECT_TRUE(buildFeatureModel("test_only_parents.xml"));
}

TEST(FeatureModelParser, outOfOrder) {
  EXPECT_TRUE(buildFeatureModel("test_out_of_order.xml"));
}

TEST(FeatureModelParser, errorMismatchParentChild) {
  EXPECT_FALSE(buildFeatureModel("error_mismatch_parent_child.xml"));
}

TEST(FeatureModelParser, errorMissingChild) {
  EXPECT_FALSE(buildFeatureModel("error_missing_child.xml"));
}

TEST(FeatureModelParser, errorMissingExclude) {
  EXPECT_FALSE(buildFeatureModel("error_missing_exclude.xml"));
}

TEST(FeatureModelParser, errorMissingImplication) {
  EXPECT_FALSE(buildFeatureModel("error_missing_implication.xml"));
}

TEST(FeatureModelParser, errorMissingParent) {
  EXPECT_FALSE(buildFeatureModel("error_missing_parent.xml"));
}

TEST(FeatureModelParser, errorRootRoot) {
  EXPECT_FALSE(buildFeatureModel("error_root_root.xml"));
}

TEST(FeatureModelParser, longRange) {
  auto FM = buildFeatureModel("test_numbers.xml");
  ASSERT_TRUE(FM);

  if (auto *F = llvm::dyn_cast_or_null<NumericFeature>(FM->getFeature("A"));
      F) {
    ASSERT_TRUE(
        std::holds_alternative<NumericFeature::ValueRangeType>(F->getValues()));
    EXPECT_EQ((std::get<NumericFeature::ValueRangeType>(F->getValues())).first,
              std::numeric_limits<long>::min());
    EXPECT_EQ((std::get<NumericFeature::ValueRangeType>(F->getValues())).second,
              std::numeric_limits<long>::max());
  } else {
    FAIL();
  }
}

TEST(FeatureModelParser, longList) {
  auto FM = buildFeatureModel("test_numbers.xml");
  ASSERT_TRUE(FM);

  if (auto *F = llvm::dyn_cast_or_null<NumericFeature>(FM->getFeature("B"));
      F) {
    ASSERT_TRUE(
        std::holds_alternative<NumericFeature::ValueListType>(F->getValues()));
    EXPECT_THAT(std::get<NumericFeature::ValueListType>(F->getValues()),
                testing::ElementsAre(std::numeric_limits<long>::min(), 0L,
                                     std::numeric_limits<long>::max()));
  } else {
    FAIL();
  }
}

TEST(FeatureModelParser, scientific) {
  auto FM = buildFeatureModel("test_numbers.xml");
  ASSERT_TRUE(FM);

  if (auto *F = llvm::dyn_cast_or_null<NumericFeature>(FM->getFeature("C"));
      F) {
    ASSERT_TRUE(
        std::holds_alternative<NumericFeature::ValueRangeType>(F->getValues()));
    EXPECT_EQ((std::get<NumericFeature::ValueRangeType>(F->getValues())).first,
              0);
    EXPECT_EQ((std::get<NumericFeature::ValueRangeType>(F->getValues())).second,
              4000);
  } else {
    FAIL();
  }
}

TEST(FeatureModelParser, memberOffset) {
  auto FM = buildFeatureModel("test_member_offset.xml");
  ASSERT_TRUE(FM);

  auto *Feature = FM->getFeature("A");
  for (auto &Loc : Feature->getLocations()) {
    ASSERT_TRUE(Loc.hasMemberOffset());
    EXPECT_EQ(Loc.getMemberOffset()->className(), "className");
    EXPECT_EQ(Loc.getMemberOffset()->memberName(), "methodName");
  }
}

TEST(FeatureModelParser, outputString) {
  auto FM = buildFeatureModel("test_output_string.xml");
  ASSERT_TRUE(FM);

  auto *F = FM->getFeature("A");
  EXPECT_EQ(F->getOutputString(), "-a");
}

TEST(FeatureModelParser, stepFunction) {
  auto FM = buildFeatureModel("test_step_function.xml");
  ASSERT_TRUE(FM);

  if (auto *F = llvm::dyn_cast_or_null<NumericFeature>(FM->getFeature("A"));
      F) {
    auto *S = F->getStepFunction();
    ASSERT_TRUE(S);
    EXPECT_DOUBLE_EQ((*S)(12.42), 54.42);
  } else {
    FAIL();
  }
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

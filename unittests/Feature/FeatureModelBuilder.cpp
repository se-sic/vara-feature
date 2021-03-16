#include "vara/Feature/FeatureModelBuilder.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModelBuilder, addBinaryFeature) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("a");
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
}

TEST(FeatureModelBuilder, addNumericFeature) {
  FeatureModelBuilder B;

  B.makeFeature<NumericFeature>("a", std::vector<int>{1, 2, 3});
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
}

TEST(FeatureModelBuilder, addOptionalFeature) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("a", true);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->isOptional());
}

// TODO(se-passau/VaRA#701): Replace string equals with subtree comparison
TEST(FeatureModelBuilder, addExcludeConstraint) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  auto C = std::make_unique<ImpliesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<Feature>("a")),
      std::make_unique<NotConstraint>(
          std::make_unique<PrimaryFeatureConstraint>(
              std::make_unique<Feature>("b"))));
  auto Expected = C->toString();

  B.addConstraint(std::move(C));

  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(
      (*FM->getFeature("a")->constraints().begin())->getRoot()->toString(),
      Expected);
}

// TODO(se-passau/VaRA#701): Replace string equals with subtree comparison
TEST(FeatureModelBuilder, addImplicationConstraint) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  auto C = std::make_unique<ImpliesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<Feature>("a")),
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<Feature>("b")));
  auto Expected = C->toString();

  B.addConstraint(std::move(C));
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(
      (*FM->getFeature("a")->constraints().begin())->getRoot()->toString(),
      Expected);
}

// TODO(se-passau/VaRA#701): Replace string equals with subtree comparison
TEST(FeatureModelBuilder, addOrConstraint) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  auto C =
      std::make_unique<OrConstraint>(std::make_unique<PrimaryFeatureConstraint>(
                                         std::make_unique<Feature>("a")),
                                     std::make_unique<PrimaryFeatureConstraint>(
                                         std::make_unique<Feature>("b")));
  auto Expected = C->toString();

  B.addConstraint(std::move(C));
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(
      (*FM->getFeature("a")->constraints().begin())->getRoot()->toString(),
      Expected);
}

TEST(FeatureModelBuilder, addBinaryFeatureRef) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("aa");
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("a");
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
}

TEST(FeatureModelBuilder, addNumericFeatureRef) {
  FeatureModelBuilder B;

  B.makeFeature<NumericFeature>("a", std::vector<int>{1, 2, 3});
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
}

TEST(FeatureModelBuilder, detectXMLAlternativesSimple) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addEdge("a", "aa");
  B.addEdge("a", "ab");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ab", false);

  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("aa")),
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("ab"))));
  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("ab")),
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("aa"))));
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  if (auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
      R) {
    EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_ALTERNATIVE);
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("aa")));
    EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
  } else {
    FAIL();
  }
}

TEST(FeatureModelBuilder, detectXMLAlternativesBroken) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addEdge("a", "aa");
  B.addEdge("a", "ab");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ab", false);

  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("aa")),
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("ab"))));
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
}

TEST(FeatureModelBuilder, detectXMLAlternativesOutOfOrder) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.addEdge("a", "aa");
  B.addEdge("a", "ab");
  B.addEdge("a", "ac");
  B.addEdge("a", "ad");
  B.addEdge("a", "ae");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ac", false);
  B.makeFeature<BinaryFeature>("ae", false);
  B.makeFeature<BinaryFeature>("ab", false);
  B.makeFeature<BinaryFeature>("ad", false);

  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("ab")),
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("ad"))));
  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("ad")),
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<BinaryFeature>("ab"))));
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ac")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ad")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ae")));
}

} // namespace vara::feature

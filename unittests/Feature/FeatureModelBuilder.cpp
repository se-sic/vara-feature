#include <vara/Feature/FeatureModel.h>

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
  BinaryFeature AA("aa");
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  BinaryFeature A("a", false, {}, nullptr, CS);

  B.addFeature(AA);
  B.addParent("aa", "a")->addFeature(A);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
}

TEST(FeatureModelBuilder, addNumericFeatureRef) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  NumericFeature A("a", std::vector<int>{1, 2, 3}, false, {}, nullptr, CS);

  B.addFeature(AA);
  B.addParent("aa", "a")->addFeature(A);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
}

TEST(Relationship, detectXMLAlternativesSimple) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa", false);
  BinaryFeature AB("ab", false);
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  CS.insert(&AB);
  BinaryFeature A("a", false, {}, nullptr, CS);
  B.addFeature(A);
  B.addParent("aa", "a")->addFeature(AA);
  B.addParent("ab", "a")->addFeature(AB);

  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(&AA),
      std::make_unique<PrimaryFeatureConstraint>(&AB)));
  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(&AB),
      std::make_unique<PrimaryFeatureConstraint>(&AA)));

  auto FM = B.buildFeatureModel();
  assert(FM);
  auto *R = llvm::dyn_cast<Relationship>(*FM->getFeature("a")->begin());
  assert(R);

  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_TRUE(R->getKind() == Relationship::RelationshipKind::RK_ALTERNATIVE);
  EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_TRUE(R->hasEdgeTo(*FM->getFeature("ab")));
}

TEST(Relationship, detectXMLAlternativesBroken) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa", false);
  BinaryFeature AB("ab", false);
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  CS.insert(&AB);
  BinaryFeature A("a", false, {}, nullptr, CS);
  B.addFeature(A);
  B.addParent("aa", "a")->addFeature(AA);
  B.addParent("ab", "a")->addFeature(AB);

  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(&AA),
      std::make_unique<PrimaryFeatureConstraint>(&AB)));

  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
}

TEST(Relationship, detectXMLAlternativesOutOfOrder) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa", false);
  BinaryFeature AB("ab", false);
  BinaryFeature AC("ac", false);
  BinaryFeature AD("ad", false);
  BinaryFeature AE("ae", false);
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  CS.insert(&AB);
  CS.insert(&AC);
  CS.insert(&AD);
  CS.insert(&AE);
  BinaryFeature A("a", false, {}, nullptr, CS);
  B.addFeature(A);
  B.addParent("aa", "a")->addFeature(AA);
  B.addParent("ab", "a")->addFeature(AB);
  B.addParent("ac", "a")->addFeature(AC);
  B.addParent("ad", "a")->addFeature(AD);
  B.addParent("ae", "a")->addFeature(AE);

  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(&AB),
      std::make_unique<PrimaryFeatureConstraint>(&AD)));
  B.addConstraint(std::make_unique<ExcludesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(&AD),
      std::make_unique<PrimaryFeatureConstraint>(&AB)));

  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ab")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ac")));
  EXPECT_FALSE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ad")));
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("ae")));
}

} // namespace vara::feature

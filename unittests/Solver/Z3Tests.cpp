#include "vara/Solver/Z3Solver.h"

#include "z3++.h"

#include "vara/Feature/FeatureModelBuilder.h"
#include "vara/Solver/ConfigurationFactory.h"
#include "gtest/gtest.h"

namespace vara::solver {

TEST(Z3Solver, AddFeatureTest) {
  std::unique_ptr<Z3Solver> S = Z3Solver::create();
  Result E = S->addFeature("A");
  EXPECT_TRUE(E);
  E = S->addFeature("B");
  EXPECT_TRUE(E);

  std::vector<int64_t> Vector{10, 20, 30};
  S->addFeature("X", Vector);
  EXPECT_TRUE(E);

  Result F = S->hasValidConfigurations();
  EXPECT_TRUE(F);

  E = S->addFeature("A");
  EXPECT_FALSE(E);
  EXPECT_EQ(ALREADY_PRESENT, E.getError());

  E = S->addFeature("X", Vector);
  EXPECT_FALSE(E);
  EXPECT_EQ(ALREADY_PRESENT, E.getError());
}

TEST(Z3Solver, AddFeatureObjectTest) {
  std::unique_ptr<Z3Solver> S = Z3Solver::create();
  // Construct a small feature model
  vara::feature::FeatureModelBuilder B;
  B.makeRoot("root");
  B.makeFeature<vara::feature::BinaryFeature>("A", true);
  B.addEdge("root", "A");
  std::vector<int64_t> Vector{0, 1, 2};
  B.makeFeature<vara::feature::NumericFeature>(
      "B", vara::feature::NumericFeature::ValueListType(Vector), false);
  B.addEdge("root", "B");
  B.makeFeature<vara::feature::BinaryFeature>("C", false);
  B.addEdge("A", "C");
  auto FM = B.buildFeatureModel();
  auto E = S->addFeature(*FM->getFeature("root"));
  EXPECT_TRUE(E);
  auto V = S->hasValidConfigurations();
  EXPECT_TRUE(V);
  EXPECT_TRUE(V.extractValue());
  E = S->addFeature(*FM->getFeature("A"));
  EXPECT_TRUE(E);
  V = S->hasValidConfigurations();
  EXPECT_TRUE(V);
  EXPECT_TRUE(V.extractValue());

  E = S->addFeature(*FM->getFeature("B"));
  EXPECT_TRUE(E);
  V = S->hasValidConfigurations();
  EXPECT_TRUE(V);
  EXPECT_TRUE(V.extractValue());
  E = S->addFeature(*FM->getFeature("B"));
  EXPECT_FALSE(E);
  EXPECT_EQ(SolverErrorCode::ALREADY_PRESENT, E.getError());

  E = S->addFeature(*FM->getFeature("C"));
  EXPECT_TRUE(E);
  V = S->hasValidConfigurations();
  EXPECT_TRUE(V.extractValue());
}

TEST(Z3Solver, TestGetNextConfiguration) {
  std::unique_ptr<Z3Solver> S = Z3Solver::create();
  vara::feature::FeatureModelBuilder B;
  B.makeRoot("root");
  B.makeFeature<vara::feature::BinaryFeature>("Foo", true);
  B.addEdge("root", "Foo");
  std::vector<int64_t> Values{0, 1};
  B.makeFeature<vara::feature::NumericFeature>("Num1", Values);
  auto FM = B.buildFeatureModel();

  S->addFeature(*FM->getFeature("root"));
  S->addFeature(*FM->getFeature("Foo"));
  S->addFeature(*FM->getFeature("Num1"));

  for (int Count = 0; Count < 4; Count++) {
    auto C = S->getNextConfiguration();
    EXPECT_TRUE(C);
    auto Config = C.extractValue();
    EXPECT_TRUE(Config->configurationOptionValue("root").has_value());
    EXPECT_TRUE(Config->configurationOptionValue("Foo").has_value());
    EXPECT_TRUE(Config->configurationOptionValue("Num1").has_value());
  }
  auto C = S->getCurrentConfiguration();
  EXPECT_TRUE(C);
  auto Config = C.extractValue();
  EXPECT_TRUE(Config->configurationOptionValue("root").has_value());
  EXPECT_TRUE(Config->configurationOptionValue("Foo").has_value());
  EXPECT_TRUE(Config->configurationOptionValue("Num1").has_value());
  auto E = S->getNextConfiguration();
  EXPECT_FALSE(E);
}

TEST(Z3Solver, AddImpliesConstraint) {
  std::unique_ptr<Z3Solver> S = Z3Solver::create();
  vara::feature::FeatureModelBuilder B;
  B.makeRoot("root");
  B.makeFeature<vara::feature::BinaryFeature>("Foo", true);
  B.addEdge("root", "Foo");
  B.makeFeature<vara::feature::BinaryFeature>("alt", false);
  B.addEdge("root", "alt");
  B.makeFeature<feature::BinaryFeature>("a", true);
  B.makeFeature<feature::BinaryFeature>("b", true);
  B.addEdge("alt", "a");
  B.addEdge("alt", "b");
  auto C = std::make_unique<feature::ImpliesConstraint>(
      std::make_unique<feature::PrimaryFeatureConstraint>(
          std::make_unique<feature::Feature>("a")),
      std::make_unique<feature::NotConstraint>(
          std::make_unique<feature::PrimaryFeatureConstraint>(
              std::make_unique<feature::Feature>("b"))));
  auto FM = B.buildFeatureModel();
  S->addFeature(*FM->getFeature("root"));
  S->addFeature(*FM->getFeature("Foo"));
  S->addFeature(*FM->getFeature("alt"));
  S->addFeature(*FM->getFeature("a"));
  S->addFeature(*FM->getFeature("b"));
  S->addConstraint(*C);
  auto I = ConfigurationIterable(std::move(S));
  EXPECT_TRUE(*I.begin());
  EXPECT_EQ(std::distance(I.begin(), I.end()), 6);
}

TEST(Z3Solver, AddAlternative) {
  std::unique_ptr<Z3Solver> S = Z3Solver::create();
  vara::feature::FeatureModelBuilder B;
  B.makeRoot("root");
  B.makeFeature<vara::feature::BinaryFeature>("A", false)->addEdge("root", "A");
  B.makeFeature<vara::feature::BinaryFeature>("A1", true)->addEdge("A", "A1");
  B.makeFeature<vara::feature::BinaryFeature>("A2", true)->addEdge("A", "A2");
  B.makeFeature<vara::feature::BinaryFeature>("A3", true)->addEdge("A", "A3");
  B.emplaceRelationship(
      vara::feature::Relationship::RelationshipKind::RK_ALTERNATIVE, "A");
  B.makeFeature<vara::feature::BinaryFeature>("B", false)->addEdge("root", "B");
  B.makeFeature<vara::feature::BinaryFeature>("B1", true)->addEdge("B", "B1");
  B.makeFeature<vara::feature::BinaryFeature>("B2", true)->addEdge("B", "B2");
  B.makeFeature<vara::feature::BinaryFeature>("B3", true)->addEdge("B", "B3");
  B.emplaceRelationship(
      vara::feature::Relationship::RelationshipKind::RK_ALTERNATIVE, "B");
  B.makeFeature<vara::feature::BinaryFeature>("C", false)->addEdge("root", "C");
  B.makeFeature<vara::feature::BinaryFeature>("C1", true)->addEdge("C", "C1");
  B.makeFeature<vara::feature::BinaryFeature>("C2", true)->addEdge("C", "C2");
  B.makeFeature<vara::feature::BinaryFeature>("C3", true)->addEdge("C", "C3");
  B.emplaceRelationship(vara::feature::Relationship::RelationshipKind::RK_OR,
                        "C");
  const std::unique_ptr<const feature::FeatureModel> FM = B.buildFeatureModel();
  S->addFeature(*FM->getFeature("root"));
  S->addFeature(*FM->getFeature("A"));
  S->addFeature(*FM->getFeature("A1"));
  S->addFeature(*FM->getFeature("A2"));
  S->addFeature(*FM->getFeature("A3"));
  S->addFeature(*FM->getFeature("B"));
  S->addFeature(*FM->getFeature("B1"));
  S->addFeature(*FM->getFeature("B2"));
  S->addFeature(*FM->getFeature("B3"));
  S->addFeature(*FM->getFeature("C"));
  S->addFeature(*FM->getFeature("C1"));
  S->addFeature(*FM->getFeature("C2"));
  S->addFeature(*FM->getFeature("C3"));

  for (const auto &R : FM->relationships()) {
    S->addRelationship(*R);
  }

  auto I = ConfigurationIterable(std::move(S));
  EXPECT_TRUE(*I.begin());
  EXPECT_EQ(std::distance(I.begin(), I.end()), 63);
}

} // namespace vara::solver

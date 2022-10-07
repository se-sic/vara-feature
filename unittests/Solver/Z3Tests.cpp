#include "vara/Solver/Solver.h"

#include "z3++.h"

#include "vara/Feature/FeatureModelBuilder.h"
#include "gtest/gtest.h"

namespace vara::solver {

TEST(Z3Solver, AddFeatureTest) {
  Z3Solver S = Z3Solver::create();
  Result E = S.addFeature("A");
  EXPECT_TRUE(E);
  E = S.addFeature("B");
  EXPECT_TRUE(E);

  std::vector<int64_t> Vector{10, 20, 30};
  S.addFeature("X", Vector);
  EXPECT_TRUE(E);

  Result F = S.hasValidConfigurations();
  EXPECT_TRUE(F);

  E = S.addFeature("A");
  EXPECT_FALSE(E);
  EXPECT_EQ(ALREADY_PRESENT, E.getError());

  E = S.addFeature("X", Vector);
  EXPECT_FALSE(E);
  EXPECT_EQ(ALREADY_PRESENT, E.getError());
}

TEST(Z3Solver, AddFeatureObjectTest) {
  Z3Solver S = Z3Solver::create();
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
  auto E = S.addFeature(*FM->getFeature("root"));
  EXPECT_TRUE(E);
  auto V = S.hasValidConfigurations();
  EXPECT_TRUE(V);
  EXPECT_TRUE(*V.extractValue());
  E = S.addFeature(*FM->getFeature("A"));
  EXPECT_TRUE(E);
  V = S.hasValidConfigurations();
  EXPECT_TRUE(V);
  EXPECT_TRUE(*V.extractValue());
  // Enumerate the solutions
  auto Enumerate = S.getNumberValidConfigurations();
  EXPECT_TRUE(Enumerate);
  EXPECT_EQ(2, *Enumerate.extractValue());

  E = S.addFeature(*FM->getFeature("B"));
  EXPECT_TRUE(E);
  V = S.hasValidConfigurations();
  EXPECT_TRUE(V);
  EXPECT_TRUE(*V.extractValue());
  E = S.addFeature(*FM->getFeature("B"));
  EXPECT_FALSE(E);
  EXPECT_EQ(SolverErrorCode::ALREADY_PRESENT, E.getError());

  // Enumerate the solutions
  Enumerate = S.getNumberValidConfigurations();
  EXPECT_TRUE(Enumerate);
  EXPECT_EQ(6, *Enumerate.extractValue());

  E = S.addFeature(*FM->getFeature("C"));
  EXPECT_TRUE(E);
  V = S.hasValidConfigurations();
  EXPECT_TRUE(*V.extractValue());
  Enumerate = S.getNumberValidConfigurations();
  EXPECT_TRUE(Enumerate);
  EXPECT_EQ(6, *Enumerate.extractValue());
}

TEST(Z3Solver, TestAllValidConfigurations) {
  Z3Solver S = Z3Solver::create();
  vara::feature::FeatureModelBuilder B;
  B.makeRoot("root");
  B.makeFeature<vara::feature::BinaryFeature>("Foo", true);
  B.addEdge("root", "Foo");
  std::vector<int64_t> Values{0, 1};
  B.makeFeature<vara::feature::NumericFeature>("Num1", Values);
  auto FM = B.buildFeatureModel();

  S.addFeature(*FM->getFeature("root"));
  S.addFeature(*FM->getFeature("Foo"));
  S.addFeature(*FM->getFeature("Num1"));

  auto C = S.getAllValidConfigurations();
  EXPECT_TRUE(C);
  EXPECT_EQ(C.extractValue()->size(), 4);
}

TEST(Z3Solver, TestGetNextConfiguration) {
  Z3Solver S = Z3Solver::create();
  vara::feature::FeatureModelBuilder B;
  B.makeRoot("root");
  B.makeFeature<vara::feature::BinaryFeature>("Foo", true);
  B.addEdge("root", "Foo");
  std::vector<int64_t> Values{0, 1};
  B.makeFeature<vara::feature::NumericFeature>("Num1", Values);
  auto FM = B.buildFeatureModel();

  S.addFeature(*FM->getFeature("root"));
  S.addFeature(*FM->getFeature("Foo"));
  S.addFeature(*FM->getFeature("Num1"));

  for (int Count = 0; Count < 4; Count++) {
    auto C = S.getNextConfiguration();
    EXPECT_TRUE(C);
    auto Config = C.extractValue();
    EXPECT_TRUE(Config->configurationOptionValue("root").hasValue());
    EXPECT_TRUE(Config->configurationOptionValue("Foo").hasValue());
    EXPECT_TRUE(Config->configurationOptionValue("Num1").hasValue());
  }
  auto E = S.getNextConfiguration();
  EXPECT_FALSE(E);
}

} // namespace vara::solver

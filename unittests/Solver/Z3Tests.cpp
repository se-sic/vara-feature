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
}

} // namespace vara::solver

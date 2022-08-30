#include "vara/Solver/Solver.h"

#include "z3++.h"

#include "gtest/gtest.h"

namespace vara::solver {

TEST(Z3Solver, AddFeatureTest) {
  Z3Solver S;
  Result E = S.addFeature("A");
  EXPECT_TRUE(E);
  E = S.addFeature("B");
  EXPECT_TRUE(E);

  std::vector<int64_t> Vector{10, 20, 30};
  S.addFeature("X", Vector);
  EXPECT_TRUE(E);

  Result F = S.hasValidConfigurations();
  EXPECT_TRUE(F);
  EXPECT_TRUE(F.extractValue());

  E = S.addFeature("A");
  EXPECT_FALSE(E);
  EXPECT_EQ(ALREADY_PRESENT, E.getError());

  E = S.addFeature("X", Vector);
  EXPECT_FALSE(E);
  EXPECT_EQ(ALREADY_PRESENT, E.getError());
}

} // namespace vara::solver

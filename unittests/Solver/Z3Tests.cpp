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

  EXPECT_TRUE(S.hasValidConfigurations());
  E = S.addFeature("A");
  EXPECT_FALSE(E);
  EXPECT_EQ(ALREADY_PRESENT, E.getError());
}

}

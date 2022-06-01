#include "vara/Solver/Solver.h"

#include "gtest/gtest.h"
#include "z3++.h"

namespace vara::solver {

TEST(Solver, Z3BasicTest) {
  z3::context C;

  z3::expr A = C.bool_const("A");
  z3::expr B = C.bool_const("B");
  z3::expr Expression = A && B;

  z3::solver S(C);

  S.add(Expression);
  EXPECT_EQ(S.check(), z3::check_result::sat);

  z3::expr InvalidatingExpression = !A;
  S.add(InvalidatingExpression);
  EXPECT_EQ(S.check(), z3::check_result::unsat);
}

} // namespace vara::solver

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

TEST(Solver, RetrieveSatisfyingModel) {
  z3::context Con;

  z3::expr A = Con.bool_const("A");
  z3::expr B = Con.bool_const("B");
  z3::expr C = Con.bool_const("C");
  z3::expr D = Con.bool_const("D");

  z3::expr Expression = A && B && !C && D;

  z3::solver S(Con);

  S.add(Expression);
  EXPECT_EQ(S.check(), z3::check_result::sat);

  z3::model M = S.get_model();

  // Retrieve the model by using the declaration function of each expression
  EXPECT_TRUE(M.get_const_interp(A.decl()));
  EXPECT_TRUE(M.get_const_interp(B.decl()));
  EXPECT_FALSE(M.get_const_interp(C.decl()));
  EXPECT_TRUE(M.get_const_interp(D.decl()));

  // Add the current configuration as a constraint
  S.add(!(A && B && !C && D));
  EXPECT_EQ(S.check(), z3::unsat);
}

TEST(Solver, RetrieveAllSatisfyingModels) {
  z3::context Con;

  z3::expr_vector Variables(Con);
  Variables.push_back(Con.bool_const("A"));
  Variables.push_back(Con.bool_const("B"));
  z3::expr A = Variables[0];
  z3::expr B = Variables[1];
  z3::solver S(Con);

  S.add(A);

  unsigned Counter = 0;
  while(S.check() == z3::sat) {
    z3::model Model = S.get_model();
    z3::expr Tmp = Con.bool_val(false);
    for (auto && Variable : Variables) {
      if (Model.get_const_interp(Variable.decl())) {
        Tmp = Tmp || !Variable;
      } else {
        Tmp = Tmp || Variable;
      }
    }
    S.add(Tmp);
    Counter++;
  }

  EXPECT_EQ(Counter, 2);

}

TEST(Solver, EquationSystem) {
  z3::context Con;

  z3::expr_vector Variables(Con);
  Variables.push_back(Con.int_const("X"));
  Variables.push_back(Con.int_const("Y"));
  Variables.push_back(Con.int_const("Z"));
  z3::expr X = Variables[0];
  z3::expr Y = Variables[1];
  z3::expr Z = Variables[2];

  z3::solver S(Con);

  S.add(5 <= X + Y + Z);
  S.add(X + Y + Z <= 16);
  S.add(-4 <= X - Y);
  S.add(X - Y <= 6);
  S.add(-1 <= X - Z);
  S.add(X - Z <= 7);
  S.add(X >= 0);
  S.add(Y >= 0);
  S.add(Z >= 0);

  unsigned Counter = 0;
  while (S.check() == z3::sat) {
    z3::model Model = S.get_model();
    z3::expr Tmp = Con.bool_val(false);
    for(auto && Variable : Variables) {
      Tmp = Tmp || (Variable != Model.eval(Variable));
    }
    S.add(Tmp);
    Counter++;
  }
  EXPECT_EQ(Counter, 313);
}

TEST(Solver, AddFeature) {

}

} // namespace vara::solver

#include "vara/Feature/ConstraintBuilder.h"
#include "vara/Feature/ConstraintParser.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(ConstraintBuilder, build) {
  auto CB = ConstraintBuilder();

  CB.feature("A");

  EXPECT_TRUE(CB.build());
}

TEST(ConstraintBuilder, none) {
  auto CB = ConstraintBuilder();

  EXPECT_FALSE(CB.build());
}

TEST(ConstraintBuilder, empty) {
  auto CB = ConstraintBuilder();

  CB.openPar().closePar()();

  EXPECT_FALSE(CB.build());
}

TEST(ConstraintBuilder, error) {
  auto CB = ConstraintBuilder();

  CB.feature("A").implies().excludes().feature("B");

  EXPECT_FALSE(CB.build());
}

class ConstraintBuilderTest : public ::testing::Test {
protected:
  void TearDown() override {
    auto Actual = CB.build();
    ASSERT_TRUE(Actual);

    EXPECT_EQ(Actual->toString(), Expected);
  }

  std::string Expected;
  ConstraintBuilder CB;
};

TEST_F(ConstraintBuilderTest, steps) {
  Expected = "(((A => !B) => !!C) => D)";
  CB.feature("A");
  CB.implies();
  CB.lNot();
  CB.feature("B");
  CB.implies();
  CB.lNot();
  CB.lNot();
  CB.feature("C");
  CB.implies();
  CB.feature("D");
}

TEST_F(ConstraintBuilderTest, chain) {
  Expected = "(((A => !B) => !!C) => D)";
  CB.feature("A")
      .implies()
      .lNot()
      .feature("B")
      .implies()
      .lNot()
      .lNot()
      .feature("C")
      .implies()
      .feature("D");
}

TEST_F(ConstraintBuilderTest, precedence) {
  Expected = "(((((A * ~B) + C) - (!!D / E)) => F) <=> (G => !(H | (I & J))))";
  CB.feature("A")
      .multiply()
      .neg()
      .feature("B")
      .add()
      .feature("C")
      .subtract()
      .lNot()
      .lNot()
      .feature("D")
      .divide()
      .feature("E")
      .implies()
      .feature("F")
      .equivalent()
      .feature("G")
      .excludes()
      .feature("H")
      .lOr()
      .feature("I")
      .lAnd()
      .feature("J");
}

TEST_F(ConstraintBuilderTest, parenthesize) {
  Expected = "A";
  CB.openPar().feature("A").closePar()();
}

TEST_F(ConstraintBuilderTest, parenthesizeUnary) {
  Expected = "!(!!A => B)";
  CB.lNot()
      .openPar()
      .lNot()
      .lNot()
      .feature("A")
      .implies()
      .feature("B")
      .closePar();
}

TEST_F(ConstraintBuilderTest, parenthesizeOperator) {
  Expected = "(((A + B) * C) + D)";
  CB.feature("A").add().feature("B")().multiply().feature("C").add().feature(
      "D");
}

TEST_F(ConstraintBuilderTest, parenthesizeBinary) {
  Expected = "(((A + B) * C) + D)";
  CB.openPar()
      .feature("A")
      .add()
      .feature("B")
      .closePar()
      .multiply()
      .feature("C")
      .add()
      .feature("D");
}

TEST_F(ConstraintBuilderTest, parenthesizeFull) {
  Expected = "((A => (!!B => !(!C => !!D))) => E)";
  CB.openPar()
      .openPar()
      .feature("A")
      .implies()
      .openPar()
      .openPar()
      .lNot()
      .lNot()
      .feature("B")
      .closePar()
      .implies()
      .openPar()
      .lNot()
      .openPar()
      .openPar()
      .lNot()
      .feature("C")
      .closePar()
      .implies()
      .lNot()
      .lNot()
      .feature("D")
      .closePar()
      .closePar()
      .closePar()
      .closePar()
      .implies()
      .feature("E")
      .closePar();
}

TEST_F(ConstraintBuilderTest, constant) {
  Expected = "42";
  CB.constant(42);
}

TEST_F(ConstraintBuilderTest, feature) {
  Expected = "Foo";
  CB.feature("Foo");
}

TEST_F(ConstraintBuilderTest, lNot) {
  Expected = "!Foo";
  CB.lNot().feature("Foo");
}

TEST_F(ConstraintBuilderTest, lOr) {
  Expected = "(Foo | Bar)";
  CB.feature("Foo").lOr().feature("Bar");
}

TEST_F(ConstraintBuilderTest, lXor) {
  Expected = "(Foo ^ Bar)";
  CB.feature("Foo").lXor().feature("Bar");
}

TEST_F(ConstraintBuilderTest, lAnd) {
  Expected = "(Foo & Bar)";
  CB.feature("Foo").lAnd().feature("Bar");
}

TEST_F(ConstraintBuilderTest, implies) {
  Expected = "(Foo => Bar)";
  CB.feature("Foo").implies().feature("Bar");
}

TEST_F(ConstraintBuilderTest, excludes) {
  Expected = "(Foo => !Bar)";
  CB.feature("Foo").excludes().feature("Bar");
}

TEST_F(ConstraintBuilderTest, equivalent) {
  Expected = "(Foo <=> Bar)";
  CB.feature("Foo").equivalent().feature("Bar");
}

TEST_F(ConstraintBuilderTest, equal) {
  Expected = "(Foo = Bar)";
  CB.feature("Foo").equal().feature("Bar");
}

TEST_F(ConstraintBuilderTest, notEqual) {
  Expected = "(Foo != Bar)";
  CB.feature("Foo").notEqual().feature("Bar");
}

TEST_F(ConstraintBuilderTest, less) {
  Expected = "(Foo < Bar)";
  CB.feature("Foo").less().feature("Bar");
}

TEST_F(ConstraintBuilderTest, greater) {
  Expected = "(Foo > Bar)";
  CB.feature("Foo").greater().feature("Bar");
}

TEST_F(ConstraintBuilderTest, lessEqual) {
  Expected = "(Foo <= Bar)";
  CB.feature("Foo").lessEqual().feature("Bar");
}

TEST_F(ConstraintBuilderTest, greaterEqual) {
  Expected = "(Foo >= Bar)";
  CB.feature("Foo").greaterEqual().feature("Bar");
}

TEST_F(ConstraintBuilderTest, neg) {
  Expected = "~Foo";
  CB.neg().feature("Foo");
}

TEST_F(ConstraintBuilderTest, add) {
  Expected = "(Foo + Bar)";
  CB.feature("Foo").add().feature("Bar");
}

TEST_F(ConstraintBuilderTest, subtract) {
  Expected = "(Foo - Bar)";
  CB.feature("Foo").subtract().feature("Bar");
}

TEST_F(ConstraintBuilderTest, multiply) {
  Expected = "(Foo * Bar)";
  CB.feature("Foo").multiply().feature("Bar");
}

TEST_F(ConstraintBuilderTest, divide) {
  Expected = "(Foo / Bar)";
  CB.feature("Foo").divide().feature("Bar");
}

} // namespace vara::feature

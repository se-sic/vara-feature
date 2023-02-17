#include "vara/Feature/ConstraintBuilder.h"
#include "vara/Feature/ConstraintParser.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(ConstraintBuilder, build) {
  auto CB = ConstraintBuilder();
  CB.feature("A").implies().lNot().feature("B")();
  CB.implies().lNot().feature("C").implies().feature("D")();
  CB.implies().feature("E");

  EXPECT_TRUE(CB.build());
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

TEST_F(ConstraintBuilderTest, parenthesizeUnary) {
  Expected = "!(!!A => B)";
  CB.lNot().left().lNot().lNot().feature("A").right().implies().feature("B");
}

TEST_F(ConstraintBuilderTest, parenthesizeBinary) {
  Expected = "((A => B) => (C => D))";
  CB.left()
      .feature("A")
      .implies()
      .feature("B")
      .right()
      .implies()
      .feature("C")
      .implies()
      .feature("D");
}

TEST_F(ConstraintBuilderTest, parenthesizeFull) {
  Expected = "((A => (!!B => !(!C => !!D))) => E)";
  CB.left()
      .left()
      .feature("A")
      .implies()
      .left()
      .left()
      .lNot()
      .lNot()
      .feature("B")
      .right()
      .implies()
      .left()
      .lNot()
      .left()
      .left()
      .lNot()
      .feature("C")
      .right()
      .implies()
      .lNot()
      .lNot()
      .feature("D")
      .right()
      .right()
      .right()
      .right()
      .implies()
      .feature("E")
      .right();
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

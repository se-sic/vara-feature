#include "vara/Feature/ConstraintBuilder.h"
#include "vara/Feature/ConstraintParser.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(ConstraintBuilder, build) {
  auto CB = ConstraintBuilder();
  CB.feature("A").implies().lNot().feature("Foo")();
  CB.implies().lNot().feature("Bar").implies().feature("D")();
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
    auto Expected = ConstraintParser(Input).buildConstraint();
    ASSERT_TRUE(Expected);

    auto Actual = CB.build();
    ASSERT_TRUE(Actual);

    EXPECT_EQ(Actual->toString(), Expected->toString());
  }

  std::string Input;
  ConstraintBuilder CB;
};

TEST_F(ConstraintBuilderTest, parenthesizeUnary) {
  Input = "!((!!A) => B)";
  CB.lNot().left().lNot().lNot().feature("A").right().implies().feature("B");
}

TEST_F(ConstraintBuilderTest, parenthesizeBinary) {
  Input = "((A => B) => (C => D))";
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
  Input = "((A => ((!!B) => (!((!C) => !!D)))) => E)";
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

} // namespace vara::feature

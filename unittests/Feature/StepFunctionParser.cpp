#include "vara/Feature/StepFunctionParser.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(StepFunctionLexer, error) {
  StepFunctionLexer L("x# 42");

  auto TokenList = L.tokenize();
  ASSERT_EQ(TokenList.size(), 2);

  EXPECT_EQ(TokenList[0].getKind(), StepFunctionToken::TokenKind::IDENTIFIER);
  EXPECT_EQ(*TokenList[0].getValue(), "x");
  EXPECT_EQ(TokenList[1].getKind(), StepFunctionToken::TokenKind::ERROR);
  EXPECT_EQ(*TokenList[1].getValue(), "#");
}

TEST(StepFunctionLexer, end) {
  StepFunctionLexer L("x\0 42"); // NOLINT

  auto TokenList = L.tokenize();
  ASSERT_EQ(TokenList.size(), 2);

  EXPECT_EQ(TokenList[0].getKind(), StepFunctionToken::TokenKind::IDENTIFIER);
  EXPECT_EQ(*TokenList[0].getValue(), "x");
  EXPECT_EQ(TokenList[1].getKind(), StepFunctionToken::TokenKind::END_OF_FILE);
}

class StepFunctionLexerTest : public ::testing::Test {
protected:
  static void checkPrimary(StepFunctionToken::TokenKind Kind,
                           const std::string &Repr) {
    auto TokenList = StepFunctionLexer(Repr).tokenize();
    ASSERT_EQ(TokenList.size(), 2);

    EXPECT_EQ(TokenList[0].getKind(), Kind);
    EXPECT_EQ(TokenList[1].getKind(),
              StepFunctionToken::TokenKind::END_OF_FILE);
  }

  static void checkBinary(StepFunctionToken::TokenKind Kind,
                          const std::string &Repr) {
    auto TokenList = StepFunctionLexer("x" + Repr + "42").tokenize();
    ASSERT_EQ(TokenList.size(), 4);

    EXPECT_EQ(TokenList[0].getKind(), StepFunctionToken::TokenKind::IDENTIFIER);
    EXPECT_EQ(*TokenList[0].getValue(), "x");
    EXPECT_EQ(TokenList[1].getKind(), Kind);
    EXPECT_EQ(TokenList[2].getKind(), StepFunctionToken::TokenKind::NUMBER);
    EXPECT_EQ(*TokenList[2].getValue(), "42");
    EXPECT_EQ(TokenList[3].getKind(),
              StepFunctionToken::TokenKind::END_OF_FILE);
  }
};

TEST_F(StepFunctionLexerTest, tokenize) {
  checkPrimary(StepFunctionToken::TokenKind::IDENTIFIER, "x");
  checkPrimary(StepFunctionToken::TokenKind::NUMBER, "42");
  checkPrimary(StepFunctionToken::TokenKind::NUMBER, "0.5");
  checkPrimary(StepFunctionToken::TokenKind::NUMBER, ".5");
  checkPrimary(StepFunctionToken::TokenKind::WHITESPACE, " ");
  checkPrimary(StepFunctionToken::TokenKind::WHITESPACE, "\n");
  checkPrimary(StepFunctionToken::TokenKind::WHITESPACE, "\r");
  checkPrimary(StepFunctionToken::TokenKind::WHITESPACE, "\t");
  checkBinary(StepFunctionToken::TokenKind::PLUS, "+");
  checkBinary(StepFunctionToken::TokenKind::STAR, "*");
  checkBinary(StepFunctionToken::TokenKind::CARET, "^");
}

TEST(StepFunctionParser, error) {
  EXPECT_FALSE(StepFunctionParser("x 42").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("x#42").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("x +").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("+ x").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("x").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("1 2").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("x + 1 + 2").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("x + 1 * 2").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("x + 1 ^ 2").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("x + y").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("1 + 2").buildStepFunction());
  EXPECT_FALSE(StepFunctionParser("1 2 +").buildStepFunction());
}

class StepFunctionParserTest : public ::testing::Test {
protected:
  static void checkStepFunction(double Current, double Next,
                                const std::string &Repr) {
    auto L = StepFunctionParser(Repr).buildStepFunction();
    ASSERT_TRUE(L);

    EXPECT_DOUBLE_EQ((*L)(Current), Next);
  }

  static void checkStepFunction(double Current, double Next,
                                const std::string &Left,
                                const std::string &Right) {
    checkStepFunction(Current, Next, Left);
    checkStepFunction(Current, Next, Right);
  }
};

TEST_F(StepFunctionParserTest, expressions) {
  checkStepFunction(0, 0, "x+0", "0+x");
  checkStepFunction(-3, 2, "x+5", "5+x");
  checkStepFunction(42, 0, "x*0", "0*x");
  checkStepFunction(1.5, 3, "x*2", "2*x");
  checkStepFunction(3, 1.5, "x*.5", ".5*x");
  checkStepFunction(0, 1, "x^0", "0^x");
  checkStepFunction(3, 9, "x^2");
  checkStepFunction(3, 8, "2^x");
  checkStepFunction(9, 3, "x^.5");
  checkStepFunction(2, .25, ".5^x");
}

} // namespace vara::feature

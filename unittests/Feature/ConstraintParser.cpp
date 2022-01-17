#include "vara/Feature/ConstraintParser.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(ConstraintLexer, error) {
  ConstraintLexer L("feature_A#feature_B");

  auto TokenList = L.tokenize();
  ASSERT_EQ(TokenList.size(), 2);

  EXPECT_EQ(TokenList[0].getKind(),
            ConstraintToken::ConstraintTokenKind::IDENTIFIER);
  EXPECT_EQ(*TokenList[0].getValue(), "feature_A");
  EXPECT_EQ(TokenList[1].getKind(),
            ConstraintToken::ConstraintTokenKind::ERROR);
  EXPECT_EQ(*TokenList[1].getValue(), "#");
}

TEST(ConstraintLexer, end) {
  ConstraintLexer L("feature_A\0feature_B"); // NOLINT

  auto TokenList = L.tokenize();
  ASSERT_EQ(TokenList.size(), 2);

  EXPECT_EQ(TokenList[0].getKind(),
            ConstraintToken::ConstraintTokenKind::IDENTIFIER);
  EXPECT_EQ(*TokenList[0].getValue(), "feature_A");
  EXPECT_EQ(TokenList[1].getKind(),
            ConstraintToken::ConstraintTokenKind::END_OF_FILE);
}

TEST(ConstraintLexer, scientific) {
  ConstraintLexer L(
      llvm::formatv("~{0}0e+2", std::numeric_limits<long>::max()));

  auto TokenList = L.tokenize();
  ASSERT_EQ(TokenList.size(), 3);

  EXPECT_EQ(TokenList[0].getKind(), ConstraintToken::ConstraintTokenKind::NEG);
  EXPECT_EQ(TokenList[1].getKind(),
            ConstraintToken::ConstraintTokenKind::NUMBER);
  EXPECT_EQ(*TokenList[1].getValue(),
            llvm::formatv("{0}0e+2", std::numeric_limits<long>::max()).str());
  EXPECT_EQ(TokenList[2].getKind(),
            ConstraintToken::ConstraintTokenKind::END_OF_FILE);
}

class ConstraintLexerTest : public ::testing::Test {
protected:
  static void checkPrimary(ConstraintToken::ConstraintTokenKind Kind,
                           const std::string &Repr) {
    auto TokenList = ConstraintLexer(Repr).tokenize();
    ASSERT_EQ(TokenList.size(), 2);

    EXPECT_EQ(TokenList[0].getKind(), Kind);
    EXPECT_EQ(TokenList[1].getKind(),
              ConstraintToken::ConstraintTokenKind::END_OF_FILE);
  }

  static void checkUnary(ConstraintToken::ConstraintTokenKind Kind,
                         const std::string &Repr) {
    auto TokenList = ConstraintLexer(Repr + "feature_A").tokenize();
    ASSERT_EQ(TokenList.size(), 3);

    EXPECT_EQ(TokenList[0].getKind(), Kind);
    EXPECT_EQ(TokenList[1].getKind(),
              ConstraintToken::ConstraintTokenKind::IDENTIFIER);
    EXPECT_EQ(*TokenList[1].getValue(), "feature_A");
    EXPECT_EQ(TokenList[2].getKind(),
              ConstraintToken::ConstraintTokenKind::END_OF_FILE);
  }

  static void checkBinary(ConstraintToken::ConstraintTokenKind Kind,
                          const std::string &Repr) {
    auto TokenList =
        ConstraintLexer("feature_A" + Repr + "feature_B").tokenize();
    ASSERT_EQ(TokenList.size(), 4);

    EXPECT_EQ(TokenList[0].getKind(),
              ConstraintToken::ConstraintTokenKind::IDENTIFIER);
    EXPECT_EQ(*TokenList[0].getValue(), "feature_A");
    EXPECT_EQ(TokenList[1].getKind(), Kind);
    EXPECT_EQ(TokenList[2].getKind(),
              ConstraintToken::ConstraintTokenKind::IDENTIFIER);
    EXPECT_EQ(*TokenList[2].getValue(), "feature_B");
    EXPECT_EQ(TokenList[3].getKind(),
              ConstraintToken::ConstraintTokenKind::END_OF_FILE);
  }
};

TEST_F(ConstraintLexerTest, tokenize) {
  checkPrimary(ConstraintToken::ConstraintTokenKind::IDENTIFIER, "feature_A");
  checkPrimary(ConstraintToken::ConstraintTokenKind::L_PAR, "(");
  checkPrimary(ConstraintToken::ConstraintTokenKind::NUMBER, "1");
  checkPrimary(ConstraintToken::ConstraintTokenKind::R_PAR, ")");
  checkPrimary(ConstraintToken::ConstraintTokenKind::WHITESPACE, " ");
  checkPrimary(ConstraintToken::ConstraintTokenKind::WHITESPACE, "\n");
  checkPrimary(ConstraintToken::ConstraintTokenKind::WHITESPACE, "\r");
  checkPrimary(ConstraintToken::ConstraintTokenKind::WHITESPACE, "\t");
  checkBinary(ConstraintToken::ConstraintTokenKind::AND, "&");
  checkBinary(ConstraintToken::ConstraintTokenKind::EQUAL, "=");
  checkBinary(ConstraintToken::ConstraintTokenKind::EQUIVALENT, "<->");
  checkBinary(ConstraintToken::ConstraintTokenKind::EQUIVALENT, "<=>");
  checkBinary(ConstraintToken::ConstraintTokenKind::GREATER, ">");
  checkBinary(ConstraintToken::ConstraintTokenKind::GREATER_EQUAL, ">=");
  checkBinary(ConstraintToken::ConstraintTokenKind::IMPLIES, "->");
  checkBinary(ConstraintToken::ConstraintTokenKind::IMPLIES, "=>");
  checkBinary(ConstraintToken::ConstraintTokenKind::LESS, "<");
  checkBinary(ConstraintToken::ConstraintTokenKind::LESS_EQUAL, "<=");
  checkBinary(ConstraintToken::ConstraintTokenKind::MINUS, "-");
  checkBinary(ConstraintToken::ConstraintTokenKind::NOT_EQUAL, "!=");
  checkBinary(ConstraintToken::ConstraintTokenKind::OR, "|");
  checkBinary(ConstraintToken::ConstraintTokenKind::PLUS, "+");
  checkBinary(ConstraintToken::ConstraintTokenKind::STAR, "*");
  checkUnary(ConstraintToken::ConstraintTokenKind::NEG, "~");
  checkUnary(ConstraintToken::ConstraintTokenKind::NOT, "!");
}

TEST(ConstraintLexer, tokenizeSxfmOr) {
  ConstraintLexer L("feature_Aor or orfeature_B");

  auto TokenList = L.tokenize();
  ASSERT_EQ(TokenList.size(), 6);

  EXPECT_EQ(TokenList[0].getKind(),
            ConstraintToken::ConstraintTokenKind::IDENTIFIER);
  EXPECT_EQ(*TokenList[0].getValue(), "feature_Aor");
  EXPECT_EQ(TokenList[1].getKind(),
            ConstraintToken::ConstraintTokenKind::WHITESPACE);
  EXPECT_EQ(TokenList[2].getKind(), ConstraintToken::ConstraintTokenKind::OR);
  EXPECT_EQ(TokenList[3].getKind(),
            ConstraintToken::ConstraintTokenKind::WHITESPACE);
  EXPECT_EQ(TokenList[4].getKind(),
            ConstraintToken::ConstraintTokenKind::IDENTIFIER);
  EXPECT_EQ(*TokenList[4].getValue(), "orfeature_B");
  EXPECT_EQ(TokenList[5].getKind(),
            ConstraintToken::ConstraintTokenKind::END_OF_FILE);
}

TEST(ConstraintParser, error) {
  EXPECT_FALSE(ConstraintParser("feature_A feature_B").buildConstraint());
  EXPECT_FALSE(ConstraintParser("feature_A#feature_B").buildConstraint());
  EXPECT_FALSE(ConstraintParser("feature_A +").buildConstraint());
  EXPECT_FALSE(ConstraintParser("!").buildConstraint());
}

TEST(ConstraintParser, parenthesis) {
  EXPECT_FALSE(ConstraintParser("(feature_A))").buildConstraint());
  EXPECT_FALSE(ConstraintParser("((feature_A)").buildConstraint());
}

TEST(ConstraintParser, radix) {
  auto C = ConstraintParser("042").buildConstraint();
  ASSERT_TRUE(C);

  EXPECT_EQ(C->getKind(), Constraint::ConstraintKind::CK_INTEGER);
  EXPECT_EQ(C->toString(), "42");
}

TEST(ConstraintParser, scientific) {
  auto C = ConstraintParser("42e+0").buildConstraint();
  ASSERT_TRUE(C);

  EXPECT_EQ(C->getKind(), Constraint::ConstraintKind::CK_INTEGER);
  EXPECT_EQ(C->toString(), "42");
}

TEST(ConstraintParser, doubleClamp) {
  auto C = ConstraintParser(
               llvm::formatv("{0}e-0", std::numeric_limits<double>::max()))
               .buildConstraint();
  ASSERT_TRUE(C);

  EXPECT_EQ(C->getKind(), Constraint::ConstraintKind::CK_INTEGER);
  EXPECT_EQ(C->toString(), std::to_string(std::numeric_limits<int64_t>::max()));
}

TEST(ConstraintParser, decimalClamp) {
  auto C = ConstraintParser(
               llvm::formatv("{0}0", std::numeric_limits<int64_t>::max()))
               .buildConstraint();
  ASSERT_TRUE(C);

  EXPECT_EQ(C->getKind(), Constraint::ConstraintKind::CK_INTEGER);
  EXPECT_EQ(C->toString(), std::to_string(std::numeric_limits<int64_t>::max()));
}

class ConstraintParserTest : public ::testing::Test {
protected:
  static void checkPrimary(Constraint::ConstraintKind Kind,
                           const std::string &Repr) {
    auto C = ConstraintParser(Repr).buildConstraint();
    ASSERT_TRUE(C);

    EXPECT_EQ(C->getKind(), Kind);
  }

  static void checkExpression(Constraint::ConstraintKind Kind,
                              const std::string &Repr,
                              const std::string &Expected) {
    auto C = ConstraintParser(Repr).buildConstraint();
    ASSERT_TRUE(C);

    EXPECT_EQ(C->getKind(), Kind);
    EXPECT_EQ(C->toString(), Expected);
  }

  static void checkUnary(Constraint::ConstraintKind Kind,
                         const std::string &Repr) {
    auto C = ConstraintParser(Repr + "feature_A").buildConstraint();
    ASSERT_TRUE(C);

    EXPECT_EQ(C->getKind(), Kind);
  }

  static void checkBinary(Constraint::ConstraintKind Kind,
                          const std::string &Repr) {
    auto C =
        ConstraintParser("feature_A" + Repr + "feature_B").buildConstraint();
    ASSERT_TRUE(C);

    EXPECT_EQ(C->getKind(), Kind);
  }
};

TEST_F(ConstraintParserTest, presecende) {
  checkExpression(Constraint::ConstraintKind::CK_MULTIPLICATION, "3 * 1 * 2",
                  "((3 * 1) * 2)");
  checkExpression(Constraint::ConstraintKind::CK_ADDITION, "3 + 1 + 2",
                  "((3 + 1) + 2)");
  checkExpression(Constraint::ConstraintKind::CK_ADDITION, "3 * 1 + 2",
                  "((3 * 1) + 2)");
  checkExpression(Constraint::ConstraintKind::CK_ADDITION, "3 + 1 * 2",
                  "(3 + (1 * 2))");
}

TEST_F(ConstraintParserTest, notnot) {
  checkExpression(Constraint::ConstraintKind::CK_NOT, "!!feature_A",
                  "!!feature_A");
  checkExpression(Constraint::ConstraintKind::CK_IMPLIES,
                  "!!feature_A => !feature_A", "(!!feature_A => !feature_A)");
  checkExpression(Constraint::ConstraintKind::CK_NOT,
                  "!(!!feature_A => !feature_A)",
                  "!(!!feature_A => !feature_A)");
  checkExpression(Constraint::ConstraintKind::CK_NOT,
                  "!!(feature_A <=> feature_A)", "!!(feature_A <=> feature_A)");
}

TEST_F(ConstraintParserTest, parenthesis) {
  checkExpression(Constraint::ConstraintKind::CK_FEATURE, "(feature_A)",
                  "feature_A");
  checkExpression(Constraint::ConstraintKind::CK_ADDITION,
                  "(a => 1) + (2 + 3) * 4", "((a => 1) + ((2 + 3) * 4))");
}

TEST_F(ConstraintParserTest, end) {
  checkBinary(Constraint::ConstraintKind::CK_FEATURE, "\0"); // NOLINT
}

TEST_F(ConstraintParserTest, expressions) {
  checkBinary(Constraint::ConstraintKind::CK_ADDITION, "+");
  checkBinary(Constraint::ConstraintKind::CK_AND, "&");
  checkBinary(Constraint::ConstraintKind::CK_EQUAL, "=");
  checkBinary(Constraint::ConstraintKind::CK_EQUIVALENCE, "<->");
  checkBinary(Constraint::ConstraintKind::CK_EQUIVALENCE, "<=>");
  checkBinary(Constraint::ConstraintKind::CK_GREATER, ">");
  checkBinary(Constraint::ConstraintKind::CK_GREATER_EQUAL, ">=");
  checkBinary(Constraint::ConstraintKind::CK_IMPLIES, "->");
  checkBinary(Constraint::ConstraintKind::CK_IMPLIES, "=>");
  checkBinary(Constraint::ConstraintKind::CK_LESS, "<");
  checkBinary(Constraint::ConstraintKind::CK_LESS_EQUAL, "<=");
  checkBinary(Constraint::ConstraintKind::CK_MULTIPLICATION, "*");
  checkBinary(Constraint::ConstraintKind::CK_NOT_EQUAL, "!=");
  checkBinary(Constraint::ConstraintKind::CK_OR, "|");
  checkBinary(Constraint::ConstraintKind::CK_SUBTRACTION, "-");
  checkPrimary(Constraint::ConstraintKind::CK_FEATURE, "feature_A");
  checkPrimary(Constraint::ConstraintKind::CK_INTEGER, "1");
  checkUnary(Constraint::ConstraintKind::CK_NEG, "~");
  checkUnary(Constraint::ConstraintKind::CK_NOT, "!");
}

} // namespace vara::feature

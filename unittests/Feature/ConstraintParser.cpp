#include "vara/Feature/ConstraintParser.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(ConstraintParser, lexError) {
  ConstraintLexer L("feature_A#feature_B");

  auto TokenList = L.tokenize();

  EXPECT_EQ(TokenList.size(), 2);
  EXPECT_EQ(TokenList[1].getKind(),
            ConstraintToken::ConstraintTokenKind::ERROR);
  EXPECT_EQ(*TokenList[1].getValue(), "#");
}

TEST(ConstraintParser, lexEOF) {
  ConstraintLexer L("feature_A\0feature_B"); // NOLINT

  auto TokenList = L.tokenize();

  EXPECT_EQ(TokenList.size(), 2);
  EXPECT_EQ(TokenList[1].getKind(),
            ConstraintToken::ConstraintTokenKind::END_OF_FILE);
}

TEST(ConstraintParser, lexEquivalent) {
  ConstraintLexer L("feature_A<->feature_B");

  auto TokenList = L.tokenize();

  EXPECT_EQ(TokenList.size(), 4);
  EXPECT_EQ(TokenList[0].getKind(),
            ConstraintToken::ConstraintTokenKind::IDENTIFIER);
  EXPECT_EQ(*TokenList[0].getValue(), "feature_A");
  EXPECT_EQ(TokenList[1].getKind(),
            ConstraintToken::ConstraintTokenKind::EQUIVALENT);
  EXPECT_EQ(TokenList[2].getKind(),
            ConstraintToken::ConstraintTokenKind::IDENTIFIER);
  EXPECT_EQ(*TokenList[2].getValue(), "feature_B");
  EXPECT_EQ(TokenList[3].getKind(),
            ConstraintToken::ConstraintTokenKind::END_OF_FILE);
}

TEST(ConstraintParser, parseError) {
  EXPECT_FALSE(ConstraintParser("feature_A feature_B").buildConstraint());
}

TEST(ConstraintParser, parseEOF) {
  EXPECT_EQ(ConstraintParser("feature_A \0 => feature_B")
                .buildConstraint()
                ->toString(),
            "feature_A");
}

TEST(ConstraintParser, parseIdentifier) {
  EXPECT_EQ(ConstraintParser("feature_A").buildConstraint()->toString(),
            "feature_A");
}

TEST(ConstraintParser, parseAdd) {
  EXPECT_EQ(ConstraintParser("1 + 2").buildConstraint()->toString(), "(1 + 2)");
}

TEST(ConstraintParser, parsePresecende) {
  EXPECT_EQ(ConstraintParser("3 + 1 + 2").buildConstraint()->toString(),
            "((3 + 1) + 2)");
  EXPECT_EQ(ConstraintParser("3 * 1 * 2").buildConstraint()->toString(),
            "((3 * 1) * 2)");
  EXPECT_EQ(ConstraintParser("3 * 1 + 2").buildConstraint()->toString(),
            "((3 * 1) + 2)");
  EXPECT_EQ(ConstraintParser("3 + 1 * 2").buildConstraint()->toString(),
            "(3 + (1 * 2))");
}

TEST(ConstraintParser, parseParenthesis) {
  // TODO(s9latimm): Fix diff representation
  EXPECT_EQ(
      ConstraintParser("(a -> 1) + (2 + 3) * 4").buildConstraint()->toString(),
      "((a => 1) + ((2 + 3) * 4))");
}

} // namespace vara::feature

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

} // namespace vara::feature

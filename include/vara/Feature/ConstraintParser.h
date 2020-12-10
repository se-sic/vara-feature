#ifndef VARA_FEATURE_CONSTRAINTPARSER_H
#define VARA_FEATURE_CONSTRAINTPARSER_H

#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"

#include <iostream>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               ConstraintToken
//===----------------------------------------------------------------------===//

class ConstraintToken {
public:
  using Precedence = int;

  enum class ConstraintTokenKind {
    ERROR,
    END_OF_FILE,
    WHITESPACE,
    IDENTIFIER,
    NUMBER,
    L_PAR,
    R_PAR,
    NOT,
    OR,
    AND,
    EQUAL,
    NOT_EQUAL,
    LESS,
    GREATER,
    IMPLIES,
    EQUIVALENT,
    PLUS,
    MINUS,
    STAR,
    NEG
  };

  ConstraintToken(ConstraintTokenKind Kind) : Kind(Kind) {}
  ConstraintToken(ConstraintTokenKind Kind, std::string Value)
      : Kind(Kind), Value(Value) {}
  ConstraintToken(const ConstraintToken &) = delete;
  ConstraintToken &operator=(ConstraintToken &) = delete;
  ConstraintToken(ConstraintToken &&) = delete;
  ConstraintToken &operator=(ConstraintToken &&) = delete;
  virtual ~ConstraintToken() = default;

  [[nodiscard]] ConstraintTokenKind getKind() const { return Kind; };

  [[nodiscard]] std::optional<std::string> getValue() { return Value; }

  [[nodiscard]] Precedence calcPrecedence() const {
    switch (Kind) {
    case ConstraintTokenKind::STAR:
      return 1;
    case ConstraintTokenKind::PLUS:
    case ConstraintTokenKind::MINUS:
      return 2;
    case ConstraintTokenKind::LESS:
    case ConstraintTokenKind::GREATER:
      return 3;
    case ConstraintTokenKind::EQUAL:
    case ConstraintTokenKind::NOT_EQUAL:
      return 4;
    case ConstraintTokenKind::AND:
      return 5;
    case ConstraintTokenKind::OR:
      return 6;
    default:
      return 7;
    }
  }

private:
  const ConstraintTokenKind Kind;
  const std::optional<std::string> Value{std::nullopt};
};

//===----------------------------------------------------------------------===//
//                               ConstraintToken
//===----------------------------------------------------------------------===//

class ConstraintLexer {
public:
  using TokenListTy = std::vector<std::unique_ptr<ConstraintToken>>;

  explicit ConstraintLexer(std::string Cnt) : Cnt(std::move(Cnt)) {}

  TokenListTy buildTokenList() {
    TokenListTy TokenList;
    for (int Pos = 0; Pos < Cnt.size();) {
      auto Result = munch(Pos);
      TokenList.push_back(std::move(Result.first));
      Pos += Result.second;
      if (auto Kind = TokenList.back()->getKind();
          Kind == ConstraintToken::ConstraintTokenKind::END_OF_FILE ||
          Kind == ConstraintToken::ConstraintTokenKind::ERROR) {
        break;
      }
    }

    // ensure last token is always either EOF or an error
    if (auto Kind = TokenList.back()->getKind();
        Kind != ConstraintToken::ConstraintTokenKind::END_OF_FILE &&
        Kind != ConstraintToken::ConstraintTokenKind::ERROR) {
      TokenList.push_back(std::make_unique<ConstraintToken>(
          ConstraintToken::ConstraintTokenKind::END_OF_FILE));
    }

    return std::move(TokenList);
  }

private:
  using ResultTy = std::pair<std::unique_ptr<ConstraintToken>, int>;

  std::string Cnt;

  ResultTy munch(const int Pos) {
    // TODO(se-passau/VaRA#664)
    switch (Cnt[Pos]) {
    case EOF:
    case '\0':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::END_OF_FILE)),
              1};
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      return munchWhitespace(Pos);
    case '(':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::L_PAR)),
              1};
    case ')':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::R_PAR)),
              1};
    case '!':
    case '+':
    case '-':
    case '<':
    case '>':
    case '~':
    case '*':
    case '&':
    case '|':
      return munchOperator(Pos);
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return munchNumber(Pos);
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'i':
    case 'l':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case '_':
    case 'h':
    case 'j':
    case 'k':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
      return munchIdentifier(Pos);
    default:
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::ERROR,
                  std::string(1, Cnt[Pos]))),
              1};
    }
  }

  ResultTy munchWhitespace(const int Pos) {
    int MunchLength = 0;
    for (int LL = Pos; LL < Cnt.size(); LL++) {
      auto Head = Cnt[LL];
      if (Head == ' ' || Head == '\t' || Head == '\n' || Head == '\r') {
        MunchLength++;
      } else {
        break;
      }
    }
    return {std::move(std::make_unique<ConstraintToken>(
                ConstraintToken::ConstraintTokenKind::WHITESPACE)),
            MunchLength};
  }

  ResultTy munchOperator(const int Pos) {
    switch (Cnt[Pos]) {
    case '+':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::PLUS)),
              1};
    case '-':
      if (Pos + 1 < Cnt.size() && Cnt[Pos + 1] == '>') {
        return {std::move(std::make_unique<ConstraintToken>(
                    ConstraintToken::ConstraintTokenKind::IMPLIES)),
                2};
      }
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::MINUS)),
              1};
    case '<':
      if (Pos + 2 < Cnt.size() && Cnt[Pos + 1] == '-' && Cnt[Pos + 2] == '>') {
        return {std::move(std::make_unique<ConstraintToken>(
                    ConstraintToken::ConstraintTokenKind::EQUIVALENT)),
                3};
      }
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::LESS)),
              1};
    case '>':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::GREATER)),
              1};
    case '!':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::NOT)),
              1};
    case '~':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::NEG)),
              1};
    case '*':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::STAR)),
              1};
    case '&':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::AND)),
              1};
    case '|':
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::OR)),
              1};
    default:
      return {std::move(std::make_unique<ConstraintToken>(
                  ConstraintToken::ConstraintTokenKind::ERROR,
                  std::string(1, Cnt[Pos]))),
              1};
    }
  }

  ResultTy munchNumber(const int Pos) {
    std::stringstream Munch;
    int MunchLength = 0;
    for (int LL = Pos; LL < Cnt.size(); LL++) {
      auto Head = Cnt[LL];
      if ('0' <= Head && Head <= '9') {
        Munch << Head;
        MunchLength++;
      } else {
        break;
      }
    }
    return {std::move(std::make_unique<ConstraintToken>(
                ConstraintToken::ConstraintTokenKind::NUMBER, Munch.str())),
            MunchLength};
  }

  ResultTy munchIdentifier(const int Pos) {
    std::stringstream Munch;
    int MunchLength = 0;
    for (int LL = Pos; LL < Cnt.size(); LL++) {
      auto Head = Cnt[LL];
      if (('0' <= Head && Head <= '9') || ('a' <= Head && Head <= 'z') ||
          ('A' <= Head && Head <= 'Z') || Head == '_') {
        Munch << Head;
        MunchLength++;
      } else {
        break;
      }
    }
    return {std::move(std::make_unique<ConstraintToken>(
                ConstraintToken::ConstraintTokenKind::IDENTIFIER, Munch.str())),
            MunchLength};
  }
};

//===----------------------------------------------------------------------===//
//                               ConstraintParser
//===----------------------------------------------------------------------===//

class ConstraintParser {
public:
  explicit ConstraintParser(std::string Cnt) : Cnt(std::move(Cnt)) {}

  std::unique_ptr<Constraint> buildConstraint() {
    ConstraintLexer::TokenListTy TokenList =
        std::move(ConstraintLexer(Cnt).buildTokenList());
    // TODO(se-passau/VaRA#664)
    return nullptr;
  }

private:
  std::string Cnt;
};

} // namespace vara::feature

#endif // VARA_FEATURE_CONSTRAINTPARSER_H

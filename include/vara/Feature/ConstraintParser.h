#ifndef VARA_FEATURE_CONSTRAINTPARSER_H
#define VARA_FEATURE_CONSTRAINTPARSER_H

#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"

#include "llvm/ADT/StringExtras.h"

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
    NEG,
    STAR,
    PLUS,
    MINUS,
    LESS,
    GREATER,
    EQUAL,
    NOT_EQUAL,
    AND,
    OR,
    IMPLIES,
    EQUIVALENT
  };

  ConstraintToken(ConstraintTokenKind Kind) : Kind(Kind) {}
  ConstraintToken(ConstraintTokenKind Kind, std::string Value)
      : Kind(Kind), Value(Value) {}
  virtual ~ConstraintToken() = default;

  [[nodiscard]] ConstraintTokenKind getKind() const { return Kind; };

  [[nodiscard]] std::optional<std::string> getValue() const { return Value; }

  [[nodiscard]] Precedence calcPrecedence() const {
    switch (Kind) {
    case ConstraintTokenKind::L_PAR:
    case ConstraintTokenKind::R_PAR:
      return 1;
    case ConstraintTokenKind::NOT:
    case ConstraintTokenKind::NEG:
      return 2;
    case ConstraintTokenKind::STAR:
      return 3;
    case ConstraintTokenKind::PLUS:
    case ConstraintTokenKind::MINUS:
      return 4;
    case ConstraintTokenKind::LESS:
    case ConstraintTokenKind::GREATER:
      return 5;
    case ConstraintTokenKind::EQUAL:
    case ConstraintTokenKind::NOT_EQUAL:
      return 6;
    case ConstraintTokenKind::AND:
      return 7;
    case ConstraintTokenKind::OR:
      return 8;
    case ConstraintTokenKind::IMPLIES:
      return 9;
    case ConstraintTokenKind::EQUIVALENT:
      return 10;
    default:
      return 0;
    }
  }

private:
  const ConstraintTokenKind Kind;
  const std::optional<const std::string> Value{std::nullopt};
};

//===----------------------------------------------------------------------===//
//                               ConstraintToken
//===----------------------------------------------------------------------===//

class ConstraintLexer {
public:
  using TokenListTy = std::vector<ConstraintToken>;

  explicit ConstraintLexer(std::string Cnt) : Cnt(std::move(Cnt)) {}

  TokenListTy tokenize() {
    TokenListTy TokenList;

    for (llvm::StringRef Str = Cnt; !Str.empty();) {
      auto Result = munch(Str);
      TokenList.push_back(Result.first);
      if (auto Kind = Result.first.getKind();
          Kind == ConstraintToken::ConstraintTokenKind::END_OF_FILE ||
          Kind == ConstraintToken::ConstraintTokenKind::ERROR) {
        break;
      }
      Str = Str.drop_front(Result.second);
    }

    // ensure last token is always either EOF or an error
    if (auto Kind = TokenList.back().getKind();
        Kind != ConstraintToken::ConstraintTokenKind::END_OF_FILE &&
        Kind != ConstraintToken::ConstraintTokenKind::ERROR) {
      TokenList.push_back(
          ConstraintToken(ConstraintToken::ConstraintTokenKind::END_OF_FILE));
    }

    return TokenList;
  }

private:
  using ResultTy = std::pair<ConstraintToken, int>;

  static ResultTy munch(const llvm::StringRef &Str) {
    switch (Str.front()) {
    case EOF:
    case '\0':
      return {
          ConstraintToken(ConstraintToken::ConstraintTokenKind::END_OF_FILE),
          1};
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      return munchWhitespace(Str);
    case '(':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::L_PAR), 1};
    case ')':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::R_PAR), 1};
    case '!':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::NOT), 1};
    case '+':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::PLUS), 1};
    case '>':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::GREATER),
              1};
    case '~':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::NEG), 1};
    case '*':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::STAR), 1};
    case '&':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::AND), 1};
    case '|':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::OR), 1};
    case '-':
    case '<':
      return munchOperator(Str);
    case '0' ... '9':
      return munchNumber(Str);
    case 'a' ... 'z':
    case 'A' ... 'Z':
      return munchIdentifier(Str);
    default:
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::ERROR,
                              Str.take_front()),
              1};
    }
  }

  static ResultTy munchWhitespace(const llvm::StringRef &Str) {
    auto Munch = Str.take_while(
        [](auto C) { return C == ' ' || C == '\t' || C == '\r' || C == '\n'; });
    return {ConstraintToken(ConstraintToken::ConstraintTokenKind::WHITESPACE),
            Munch.size()};
  }

  static ResultTy munchOperator(const llvm::StringRef &Str) {
    switch (Str.front()) {
    case '-':
      if (Str.startswith("->")) {
        return {ConstraintToken(ConstraintToken::ConstraintTokenKind::IMPLIES),
                2};
      }
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::MINUS), 1};
    case '<':
      if (Str.startswith("<->")) {
        return {
            ConstraintToken(ConstraintToken::ConstraintTokenKind::EQUIVALENT),
            3};
      }
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::LESS), 1};
    default:
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::ERROR,
                              Str.take_front()),
              1};
    }
  }

  static ResultTy munchNumber(const llvm::StringRef &Str) {
    auto Munch = Str.take_while([](auto C) { return llvm::isDigit(C); });
    return {
        ConstraintToken(ConstraintToken::ConstraintTokenKind::NUMBER, Munch),
        Munch.size()};
  }

  static ResultTy munchIdentifier(const llvm::StringRef &Str) {
    auto Munch =
        Str.take_while([](auto C) { return llvm::isAlnum(C) || C == '_'; });
    return {ConstraintToken(ConstraintToken::ConstraintTokenKind::IDENTIFIER,
                            Munch),
            Munch.size()};
  }

  std::string Cnt;
};

//===----------------------------------------------------------------------===//
//                               ConstraintParser
//===----------------------------------------------------------------------===//

class ConstraintParser {
public:
  explicit ConstraintParser(std::string Cnt) : Cnt(std::move(Cnt)) {}

  std::unique_ptr<Constraint> buildConstraint() {
    ConstraintLexer::TokenListTy TokenList =
        std::move(ConstraintLexer(Cnt).tokenize());
    // TODO(se-passau/VaRA#664)
    return nullptr;
  }

private:
  std::string Cnt;
};

} // namespace vara::feature

#endif // VARA_FEATURE_CONSTRAINTPARSER_H

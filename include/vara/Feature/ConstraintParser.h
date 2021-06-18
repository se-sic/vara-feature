#ifndef VARA_FEATURE_CONSTRAINTPARSER_H
#define VARA_FEATURE_CONSTRAINTPARSER_H

#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"

#include "llvm/ADT/StringExtras.h"

#include <iostream>
#include <utility>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               ConstraintToken
//===----------------------------------------------------------------------===//

class ConstraintToken {
public:
  using PrecedenceTy = unsigned int;
  static const PrecedenceTy MaxPrecedence = 9;

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

  [[nodiscard]] PrecedenceTy calcPrecedence() const {
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
    case ConstraintTokenKind::IMPLIES:
      return 7;
    case ConstraintTokenKind::EQUIVALENT:
      return 8;
    default:
      static_assert(ConstraintToken::MaxPrecedence >= 9);
      return ConstraintToken::MaxPrecedence;
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
  using TokenListTy = std::deque<ConstraintToken>;

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
                              Str.take_front().str()),
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
                              Str.take_front().str()),
              1};
    }
  }

  static ResultTy munchNumber(const llvm::StringRef &Str) {
    auto Munch = Str.take_while([](auto C) { return llvm::isDigit(C); });
    return {ConstraintToken(ConstraintToken::ConstraintTokenKind::NUMBER,
                            Munch.str()),
            Munch.size()};
  }

  static ResultTy munchIdentifier(const llvm::StringRef &Str) {
    auto Munch =
        Str.take_while([](auto C) { return llvm::isAlnum(C) || C == '_'; });
    return {ConstraintToken(ConstraintToken::ConstraintTokenKind::IDENTIFIER,
                            Munch.str()),
            Munch.size()};
  }

  std::string Cnt;
};

//===----------------------------------------------------------------------===//
//                               ConstraintParser
//===----------------------------------------------------------------------===//

class ConstraintParser {
public:
  explicit ConstraintParser(std::string Cnt)
      : TokenList(ConstraintLexer(std::move(Cnt)).tokenize()) {}

  std::unique_ptr<Constraint> buildConstraint() { return parseExpression(); }

private:
  [[nodiscard]] const ConstraintToken &peek() const {
    return TokenList.front();
  }

  [[nodiscard]] ConstraintToken next() {
    auto Token = TokenList.front();
    TokenList.pop_front();
    return Token;
  }

  bool consume(const ConstraintToken::ConstraintTokenKind Kind) {
    if (TokenList.front().getKind() == Kind) {
      TokenList.pop_front();
      return true;
    }
    return false;
  }

  std::unique_ptr<Constraint>
  parseExpression(ConstraintToken::PrecedenceTy Precedence =
                      ConstraintToken::MaxPrecedence) {
    if (auto LHS = parseUnaryExpression()) {
      while (true) {
        switch (peek().getKind()) {
        case ConstraintToken::ConstraintTokenKind::WHITESPACE:
          consume(ConstraintToken::ConstraintTokenKind::WHITESPACE);
          continue;
        case ConstraintToken::ConstraintTokenKind::R_PAR:
        case ConstraintToken::ConstraintTokenKind::END_OF_FILE:
          return LHS;
        case ConstraintToken::ConstraintTokenKind::EQUAL:
        case ConstraintToken::ConstraintTokenKind::EQUIVALENT:
        case ConstraintToken::ConstraintTokenKind::GREATER:
        case ConstraintToken::ConstraintTokenKind::NOT_EQUAL:
        case ConstraintToken::ConstraintTokenKind::STAR:
        case ConstraintToken::ConstraintTokenKind::PLUS:
        case ConstraintToken::ConstraintTokenKind::MINUS:
        case ConstraintToken::ConstraintTokenKind::LESS:
        case ConstraintToken::ConstraintTokenKind::AND:
        case ConstraintToken::ConstraintTokenKind::OR:
        case ConstraintToken::ConstraintTokenKind::IMPLIES: {
          auto NextPrecedence = peek().calcPrecedence();
          if (NextPrecedence >= Precedence) {
            return LHS;
          }
          LHS = parseBinaryExpression(std::move(LHS), NextPrecedence);
          continue;
        }
        case ConstraintToken::ConstraintTokenKind::IDENTIFIER:
        case ConstraintToken::ConstraintTokenKind::NUMBER:
        case ConstraintToken::ConstraintTokenKind::L_PAR:
        case ConstraintToken::ConstraintTokenKind::NOT:
        case ConstraintToken::ConstraintTokenKind::NEG:
          llvm::errs() << "Unexpected token in expression.\n";
          return nullptr;
        case ConstraintToken::ConstraintTokenKind::ERROR:
          llvm::errs() << "Error.\n";
          return nullptr;
        }
      }
    }
    return nullptr;
  }

  std::unique_ptr<Constraint>
  parseBinaryExpression(std::unique_ptr<Constraint> LHS,
                        ConstraintToken::PrecedenceTy Precedence) {
    while (true) {
      switch (peek().getKind()) {
      case ConstraintToken::ConstraintTokenKind::WHITESPACE:
        consume(ConstraintToken::ConstraintTokenKind::WHITESPACE);
        continue;
      case ConstraintToken::ConstraintTokenKind::END_OF_FILE:
        return LHS;
      case ConstraintToken::ConstraintTokenKind::EQUAL:
        consume(ConstraintToken::ConstraintTokenKind::EQUAL);
        return std::make_unique<EqualsConstraint>(std::move(LHS),
                                                  parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::EQUIVALENT:
        consume(ConstraintToken::ConstraintTokenKind::EQUIVALENT);
        return std::make_unique<EquivalenceConstraint>(
            std::move(LHS), parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::GREATER:
        consume(ConstraintToken::ConstraintTokenKind::GREATER);
        return std::make_unique<GreaterConstraint>(std::move(LHS),
                                                   parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::NOT_EQUAL:
        consume(ConstraintToken::ConstraintTokenKind::NOT_EQUAL);
        return std::make_unique<NotConstraint>(
            std::make_unique<EqualsConstraint>(std::move(LHS),
                                               parseExpression(Precedence)));
      case ConstraintToken::ConstraintTokenKind::STAR:
        consume(ConstraintToken::ConstraintTokenKind::STAR);
        return std::make_unique<MultiplicationConstraint>(
            std::move(LHS), parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::PLUS:
        consume(ConstraintToken::ConstraintTokenKind::PLUS);
        return std::make_unique<AdditionConstraint>(
            std::move(LHS), parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::MINUS:
        consume(ConstraintToken::ConstraintTokenKind::MINUS);
        return std::make_unique<SubtractionConstraint>(
            std::move(LHS), parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::LESS:
        consume(ConstraintToken::ConstraintTokenKind::LESS);
        return std::make_unique<LessConstraint>(std::move(LHS),
                                                parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::AND:
        consume(ConstraintToken::ConstraintTokenKind::AND);
        return std::make_unique<AndConstraint>(std::move(LHS),
                                               parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::OR:
        consume(ConstraintToken::ConstraintTokenKind::OR);
        return std::make_unique<OrConstraint>(std::move(LHS),
                                              parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::IMPLIES:
        consume(ConstraintToken::ConstraintTokenKind::IMPLIES);
        return std::make_unique<ImpliesConstraint>(std::move(LHS),
                                                   parseExpression(Precedence));
      case ConstraintToken::ConstraintTokenKind::ERROR:
      case ConstraintToken::ConstraintTokenKind::IDENTIFIER:
      case ConstraintToken::ConstraintTokenKind::NUMBER:
      case ConstraintToken::ConstraintTokenKind::L_PAR:
      case ConstraintToken::ConstraintTokenKind::R_PAR:
      case ConstraintToken::ConstraintTokenKind::NOT:
      case ConstraintToken::ConstraintTokenKind::NEG:
        llvm::errs() << "Unexpected token in binary expression.\n";
        return nullptr;
      }
    }
  }

  std::unique_ptr<Constraint> parseUnaryExpression() {
    while (true) {
      switch (peek().getKind()) {
      case ConstraintToken::ConstraintTokenKind::WHITESPACE:
        consume(ConstraintToken::ConstraintTokenKind::WHITESPACE);
        continue;
      case ConstraintToken::ConstraintTokenKind::EQUAL:
      case ConstraintToken::ConstraintTokenKind::EQUIVALENT:
      case ConstraintToken::ConstraintTokenKind::GREATER:
      case ConstraintToken::ConstraintTokenKind::NOT_EQUAL:
      case ConstraintToken::ConstraintTokenKind::STAR:
      case ConstraintToken::ConstraintTokenKind::PLUS:
      case ConstraintToken::ConstraintTokenKind::MINUS:
      case ConstraintToken::ConstraintTokenKind::LESS:
      case ConstraintToken::ConstraintTokenKind::AND:
      case ConstraintToken::ConstraintTokenKind::OR:
      case ConstraintToken::ConstraintTokenKind::IMPLIES:
      case ConstraintToken::ConstraintTokenKind::END_OF_FILE:
        llvm::errs() << "Unexpected token before unary expression.\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::R_PAR:
        llvm::errs() << "Unexpected closing parenthesis.\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::ERROR:
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::IDENTIFIER:
        return std::make_unique<PrimaryFeatureConstraint>(
            std::make_unique<Feature>(*next().getValue()));
      case ConstraintToken::ConstraintTokenKind::NUMBER:
        return std::make_unique<PrimaryIntegerConstraint>(
            std::stoi(*next().getValue()));
      case ConstraintToken::ConstraintTokenKind::NOT:
        consume(ConstraintToken::ConstraintTokenKind::NOT);
        return std::make_unique<NotConstraint>(parseExpression());
      case ConstraintToken::ConstraintTokenKind::NEG:
        consume(ConstraintToken::ConstraintTokenKind::NEG);
        return std::make_unique<NegConstraint>(parseExpression());
      case ConstraintToken::ConstraintTokenKind::L_PAR:
        consume(ConstraintToken::ConstraintTokenKind::L_PAR);
        auto Inner = parseExpression();
        if (!consume(ConstraintToken::ConstraintTokenKind::R_PAR)) {
          llvm::errs() << "Expected closing parenthesis.\n";
          return nullptr;
        }
        return Inner;
      }
    }
  }

  ConstraintLexer::TokenListTy TokenList;
};

} // namespace vara::feature

#endif // VARA_FEATURE_CONSTRAINTPARSER_H

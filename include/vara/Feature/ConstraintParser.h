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
  static const PrecedenceTy MinPrecedence = 0;
  static const PrecedenceTy MaxPrecedence = 9;

  enum class ConstraintTokenKind {
    AND,
    END_OF_FILE,
    EQUAL,
    EQUIVALENT,
    ERROR,
    GREATER,
    GREATER_EQUAL,
    IDENTIFIER,
    IMPLIES,
    L_PAR,
    LESS,
    LESS_EQUAL,
    MINUS,
    NEG,
    NOT,
    NOT_EQUAL,
    NUMBER,
    OR,
    PLUS,
    R_PAR,
    STAR,
    WHITESPACE
  };

  ConstraintToken(ConstraintTokenKind Kind) : Kind(Kind) {}
  ConstraintToken(ConstraintTokenKind Kind, const std::string &Value)
      : Kind(Kind), Value(Value) {}
  virtual ~ConstraintToken() = default;

  [[nodiscard]] ConstraintTokenKind getKind() const { return Kind; };

  [[nodiscard]] std::optional<const std::string> getValue() const {
    return Value;
  }

  [[nodiscard]] PrecedenceTy calcPrecedence() const {
    switch (Kind) {
    case ConstraintTokenKind::STAR:
      return 1;
    case ConstraintTokenKind::PLUS:
    case ConstraintTokenKind::MINUS:
      return 2;
    case ConstraintTokenKind::GREATER:
    case ConstraintTokenKind::GREATER_EQUAL:
    case ConstraintTokenKind::LESS:
    case ConstraintTokenKind::LESS_EQUAL:
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
      static_assert(ConstraintToken::MinPrecedence < 1 &&
                    ConstraintToken::MaxPrecedence > 8);
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

  static ResultTy munch(const llvm::StringRef Str) {
    if (('a' <= Str.front() && Str.front() <= 'z') ||
        ('A' <= Str.front() && Str.front() <= 'Z')) {
      return munchIdentifier(Str);
    }
    if ('0' <= Str.front() && Str.front() <= '9') {
      return munchNumber(Str);
    }

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
    case '+':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::PLUS), 1};
    case '~':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::NEG), 1};
    case '*':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::STAR), 1};
    case '&':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::AND), 1};
    case '|':
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::OR), 1};
    case '-':
    case '!':
    case '=':
    case '>':
    case '<':
      return munchOperator(Str);
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
    case '!':
      if (Str.startswith("!=")) {
        return {
            ConstraintToken(ConstraintToken::ConstraintTokenKind::NOT_EQUAL),
            2};
      }
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::NOT), 1};
    case '=':
      if (Str.startswith("=>")) {
        return {ConstraintToken(ConstraintToken::ConstraintTokenKind::IMPLIES),
                2};
      }
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::EQUAL), 1};
    case '>':
      if (Str.startswith(">=")) {
        return {ConstraintToken(
                    ConstraintToken::ConstraintTokenKind::GREATER_EQUAL),
                2};
      }
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::GREATER),
              1};
    case '<':
      if (Str.startswith("<->") || Str.startswith("<=>")) {
        return {
            ConstraintToken(ConstraintToken::ConstraintTokenKind::EQUIVALENT),
            3};
      } else if (Str.startswith("<=")) {
        return {
            ConstraintToken(ConstraintToken::ConstraintTokenKind::LESS_EQUAL),
            2};
      }
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::LESS), 1};
    default:
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::ERROR,
                              Str.take_front().str()),
              1};
    }
  }

  static ResultTy munchNumber(const llvm::StringRef &Str) {
    auto Munch = Str.take_while([](auto C) {
      return llvm::isDigit(C) || C == 'e' || C == 'E' || C == '+' || C == '-';
    });

    return {ConstraintToken(ConstraintToken::ConstraintTokenKind::NUMBER,
                            Munch.lower()),
            Munch.size()};
  }

  static ResultTy munchIdentifier(const llvm::StringRef &Str) {
    auto Munch =
        Str.take_while([](auto C) { return llvm::isAlnum(C) || C == '_'; });

    // operator used in SXFM constraints
    if (Munch == "or") {
      return {ConstraintToken(ConstraintToken::ConstraintTokenKind::OR), 2};
    }

    return {ConstraintToken(ConstraintToken::ConstraintTokenKind::IDENTIFIER,
                            Munch.str()),
            Munch.size()};
  }

  std::string Cnt;
};

//===----------------------------------------------------------------------===//
//                               ConstraintParser
//===----------------------------------------------------------------------===//

/// Parse 64-bit integer in decimal or scientific notation.
static int64_t parseInteger(llvm::StringRef Str,
                            std::optional<unsigned int> Line = std::nullopt) {
  if (Str.contains_insensitive('e')) {
    // If we encounter scientific notation we try to parse the number as double.
    if (double Double; !Str.getAsDouble(Double)) {
      return parseInteger(llvm::formatv("{0:0}", Double).str());
    }
  } else if (int64_t Integer; !Str.getAsInteger(10, Integer)) {
    return Integer;
  }

  if (Line.has_value()) {
    llvm::errs() << "Failed to parse integer '" << Str << "' in line "
                 << Line.has_value() << ".\n";
  } else {
    llvm::errs() << "Failed to parse integer '" << Str << "'.\n";
  }

  // If parsing failed, we return minimal or maximal value respectively.
  if (Str.startswith("-")) {
    return std::numeric_limits<int64_t>::min();
  }
  return std::numeric_limits<int64_t>::max();
}

class ConstraintParser {
public:
  explicit ConstraintParser(std::string Cnt,
                            std::optional<unsigned int> Line = std::nullopt)
      : TokenList(ConstraintLexer(std::move(Cnt)).tokenize()), Line(Line) {}

  std::unique_ptr<Constraint> buildConstraint() { return parseConstraint(); }

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
  parseConstraint(int NestingLevel = 0,
                  ConstraintToken::PrecedenceTy Precedence =
                      ConstraintToken::MaxPrecedence) {
    auto LHS = parseUnaryConstraint(NestingLevel);
    while (LHS) {
      switch (peek().getKind()) {
      case ConstraintToken::ConstraintTokenKind::ERROR:
        assert(peek().getValue().has_value());
        llvm::errs() << "Lexical error: Unexpected character '"
                     << *peek().getValue() << "'\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::WHITESPACE:
        consume(ConstraintToken::ConstraintTokenKind::WHITESPACE);
        continue;
      case ConstraintToken::ConstraintTokenKind::R_PAR:
        if (NestingLevel) {
          return LHS;
        }
        llvm::errs() << "Syntax error: Unexpected closing parenthesis.\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::END_OF_FILE:
        return LHS;
      case ConstraintToken::ConstraintTokenKind::AND:
      case ConstraintToken::ConstraintTokenKind::EQUAL:
      case ConstraintToken::ConstraintTokenKind::EQUIVALENT:
      case ConstraintToken::ConstraintTokenKind::GREATER:
      case ConstraintToken::ConstraintTokenKind::GREATER_EQUAL:
      case ConstraintToken::ConstraintTokenKind::IMPLIES:
      case ConstraintToken::ConstraintTokenKind::LESS:
      case ConstraintToken::ConstraintTokenKind::LESS_EQUAL:
      case ConstraintToken::ConstraintTokenKind::MINUS:
      case ConstraintToken::ConstraintTokenKind::NOT_EQUAL:
      case ConstraintToken::ConstraintTokenKind::OR:
      case ConstraintToken::ConstraintTokenKind::PLUS:
      case ConstraintToken::ConstraintTokenKind::STAR: {
        auto NextPrecedence = peek().calcPrecedence();
        if (NextPrecedence >= Precedence) {
          return LHS;
        }
        LHS =
            parseBinaryConstraint(NestingLevel, std::move(LHS), NextPrecedence);
        continue;
      }
      case ConstraintToken::ConstraintTokenKind::IDENTIFIER:
      case ConstraintToken::ConstraintTokenKind::L_PAR:
      case ConstraintToken::ConstraintTokenKind::NEG:
      case ConstraintToken::ConstraintTokenKind::NOT:
      case ConstraintToken::ConstraintTokenKind::NUMBER:
        llvm::errs() << "Syntax error: Unexpected token in expression.\n";
        return nullptr;
      }
    }
    return nullptr;
  }

  template <typename T>
  auto createConstraint(std::unique_ptr<Constraint> LHS, int NestingLevel,
                        ConstraintToken::PrecedenceTy Precedence) {
    auto Constraint = parseConstraint(NestingLevel + 1, Precedence);
    return Constraint
               ? std::make_unique<T>(std::move(LHS), std::move(Constraint))
               : nullptr;
  }

  template <typename T>
  auto createConstraint(int NestingLevel,
                        ConstraintToken::PrecedenceTy Precedence) {
    auto Constraint = parseConstraint(NestingLevel + 1, Precedence);
    return Constraint ? std::make_unique<T>(std::move(Constraint)) : nullptr;
  }

  std::unique_ptr<Constraint>
  parseBinaryConstraint(int NestingLevel, std::unique_ptr<Constraint> LHS,
                        ConstraintToken::PrecedenceTy Precedence) {
    while (true) {
      switch (peek().getKind()) {
      case ConstraintToken::ConstraintTokenKind::ERROR:
        assert(peek().getValue().has_value());
        llvm::errs() << "Lexical error: Unexpected character '"
                     << *peek().getValue() << "'\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::END_OF_FILE:
        llvm::errs() << "Syntax error: Unexpected end of binary expression\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::WHITESPACE:
        consume(ConstraintToken::ConstraintTokenKind::WHITESPACE);
        continue;
      case ConstraintToken::ConstraintTokenKind::EQUAL:
        consume(ConstraintToken::ConstraintTokenKind::EQUAL);
        return createConstraint<EqualConstraint>(std::move(LHS),
                                                 NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::EQUIVALENT:
        consume(ConstraintToken::ConstraintTokenKind::EQUIVALENT);
        return createConstraint<EquivalenceConstraint>(
            std::move(LHS), NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::GREATER:
        consume(ConstraintToken::ConstraintTokenKind::GREATER);
        return createConstraint<GreaterConstraint>(
            std::move(LHS), NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::GREATER_EQUAL:
        consume(ConstraintToken::ConstraintTokenKind::GREATER_EQUAL);
        return createConstraint<GreaterEqualConstraint>(
            std::move(LHS), NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::NOT_EQUAL:
        consume(ConstraintToken::ConstraintTokenKind::NOT_EQUAL);
        return createConstraint<NotEqualConstraint>(
            std::move(LHS), NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::STAR:
        consume(ConstraintToken::ConstraintTokenKind::STAR);
        return createConstraint<MultiplicationConstraint>(
            std::move(LHS), NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::PLUS:
        consume(ConstraintToken::ConstraintTokenKind::PLUS);
        return createConstraint<AdditionConstraint>(
            std::move(LHS), NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::MINUS:
        consume(ConstraintToken::ConstraintTokenKind::MINUS);
        return createConstraint<SubtractionConstraint>(
            std::move(LHS), NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::LESS:
        consume(ConstraintToken::ConstraintTokenKind::LESS);
        return createConstraint<LessConstraint>(std::move(LHS),
                                                NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::LESS_EQUAL:
        consume(ConstraintToken::ConstraintTokenKind::LESS_EQUAL);
        return createConstraint<LessEqualConstraint>(
            std::move(LHS), NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::AND:
        consume(ConstraintToken::ConstraintTokenKind::AND);
        return createConstraint<AndConstraint>(std::move(LHS), NestingLevel + 1,
                                               Precedence);
      case ConstraintToken::ConstraintTokenKind::OR:
        consume(ConstraintToken::ConstraintTokenKind::OR);
        return createConstraint<OrConstraint>(std::move(LHS), NestingLevel + 1,
                                              Precedence);
      case ConstraintToken::ConstraintTokenKind::IMPLIES:
        consume(ConstraintToken::ConstraintTokenKind::IMPLIES);
        return createConstraint<ImpliesConstraint>(
            std::move(LHS), NestingLevel + 1, Precedence);
      case ConstraintToken::ConstraintTokenKind::IDENTIFIER:
      case ConstraintToken::ConstraintTokenKind::L_PAR:
      case ConstraintToken::ConstraintTokenKind::NEG:
      case ConstraintToken::ConstraintTokenKind::NOT:
      case ConstraintToken::ConstraintTokenKind::NUMBER:
      case ConstraintToken::ConstraintTokenKind::R_PAR:
        llvm::errs()
            << "Syntax error: Unexpected token in binary expression.\n";
        return nullptr;
      }
    }
  }

  std::unique_ptr<Constraint> parseUnaryConstraint(int NestingLevel) {
    while (true) {
      switch (peek().getKind()) {
      case ConstraintToken::ConstraintTokenKind::ERROR:
        assert(peek().getValue().has_value());
        llvm::errs() << "Lexical error: Unexpected character '"
                     << *peek().getValue() << "'\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::END_OF_FILE:
        llvm::errs() << "Syntax error: Unexpected end of unary expression.\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::WHITESPACE:
        consume(ConstraintToken::ConstraintTokenKind::WHITESPACE);
        continue;
      case ConstraintToken::ConstraintTokenKind::AND:
      case ConstraintToken::ConstraintTokenKind::EQUAL:
      case ConstraintToken::ConstraintTokenKind::EQUIVALENT:
      case ConstraintToken::ConstraintTokenKind::GREATER:
      case ConstraintToken::ConstraintTokenKind::GREATER_EQUAL:
      case ConstraintToken::ConstraintTokenKind::IMPLIES:
      case ConstraintToken::ConstraintTokenKind::LESS:
      case ConstraintToken::ConstraintTokenKind::LESS_EQUAL:
      case ConstraintToken::ConstraintTokenKind::MINUS:
      case ConstraintToken::ConstraintTokenKind::NOT_EQUAL:
      case ConstraintToken::ConstraintTokenKind::OR:
      case ConstraintToken::ConstraintTokenKind::PLUS:
      case ConstraintToken::ConstraintTokenKind::STAR:
        llvm::errs()
            << "Syntax error: Unexpected token before unary expression.\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::R_PAR:
        llvm::errs() << "Syntax error: Unexpected closing parenthesis.\n";
        return nullptr;
      case ConstraintToken::ConstraintTokenKind::IDENTIFIER:
        assert(peek().getValue().has_value());
        return std::make_unique<PrimaryFeatureConstraint>(
            std::make_unique<Feature>(*next().getValue()));
      case ConstraintToken::ConstraintTokenKind::NUMBER:
        assert(peek().getValue().has_value());
        return std::make_unique<PrimaryIntegerConstraint>(
            parseInteger(*next().getValue(), Line));
      case ConstraintToken::ConstraintTokenKind::NOT:
        consume(ConstraintToken::ConstraintTokenKind::NOT);
        return createConstraint<NotConstraint>(NestingLevel + 1,
                                               ConstraintToken::MinPrecedence);
      case ConstraintToken::ConstraintTokenKind::NEG:
        consume(ConstraintToken::ConstraintTokenKind::NEG);
        return createConstraint<NegConstraint>(NestingLevel + 1,
                                               ConstraintToken::MinPrecedence);
      case ConstraintToken::ConstraintTokenKind::L_PAR:
        consume(ConstraintToken::ConstraintTokenKind::L_PAR);
        auto Constraint = parseConstraint(NestingLevel + 1);
        if (!consume(ConstraintToken::ConstraintTokenKind::R_PAR)) {
          llvm::errs() << "Syntax error: Missing closing parenthesis.\n";
          return nullptr;
        }
        return Constraint;
      }
    }
  }

  ConstraintLexer::TokenListTy TokenList;
  std::optional<unsigned int> Line;
};

} // namespace vara::feature

#endif // VARA_FEATURE_CONSTRAINTPARSER_H

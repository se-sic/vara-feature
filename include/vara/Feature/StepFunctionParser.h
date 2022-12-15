#ifndef VARA_FEATURE_STEP_FUNCTION_PARSER_H
#define VARA_FEATURE_STEP_FUNCTION_PARSER_H

#include "vara/Feature/StepFunction.h"

#include <utility>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                              StepFunctionToken
//===----------------------------------------------------------------------===//

class StepFunctionToken {
public:
  enum class TokenKind {
    IDENTIFIER,
    NUMBER,
    PLUS,
    STAR,
    WHITESPACE,
    END_OF_FILE,
    ERROR
  };

  StepFunctionToken(TokenKind Kind) : Kind(Kind) {}
  StepFunctionToken(TokenKind Kind, const std::string &Value)
      : Kind(Kind), Value(Value) {}
  virtual ~StepFunctionToken() = default;

  [[nodiscard]] TokenKind getKind() const { return Kind; };

  [[nodiscard]] llvm::Optional<const std::string> getValue() const {
    return Value;
  }

private:
  const TokenKind Kind;
  const llvm::Optional<const std::string> Value{llvm::None};
};

//===----------------------------------------------------------------------===//
//                              StepFunctionLexer
//===----------------------------------------------------------------------===//

class StepFunctionLexer {
public:
  using TokenListTy = std::deque<StepFunctionToken>;

  explicit StepFunctionLexer(std::string Cnt) : Cnt(std::move(Cnt)) {}

  TokenListTy tokenize() {
    TokenListTy TokenList;

    for (llvm::StringRef Str = Cnt; !Str.empty();) {
      auto Result = munch(Str);
      TokenList.push_back(Result.first);
      if (auto Kind = Result.first.getKind();
          Kind == StepFunctionToken::TokenKind::END_OF_FILE ||
          Kind == StepFunctionToken::TokenKind::ERROR) {
        break;
      }
      Str = Str.drop_front(Result.second);
    }

    // ensure last token is always either EOF or an error
    if (auto Kind = TokenList.back().getKind();
        Kind != StepFunctionToken::TokenKind::END_OF_FILE &&
        Kind != StepFunctionToken::TokenKind::ERROR) {
      TokenList.push_back(
          StepFunctionToken(StepFunctionToken::TokenKind::END_OF_FILE));
    }

    return TokenList;
  }

private:
  using ResultTy = std::pair<StepFunctionToken, int>;

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
      return {StepFunctionToken(StepFunctionToken::TokenKind::END_OF_FILE), 1};
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      return munchWhitespace(Str);
    case '+':
      return {StepFunctionToken(StepFunctionToken::TokenKind::PLUS), 1};
    case '*':
      return {StepFunctionToken(StepFunctionToken::TokenKind::STAR), 1};
    default:
      return {StepFunctionToken(StepFunctionToken::TokenKind::ERROR,
                                Str.take_front().str()),
              1};
    }
  }

  static ResultTy munchWhitespace(const llvm::StringRef &Str) {
    auto Munch = Str.take_while(
        [](auto C) { return C == ' ' || C == '\t' || C == '\r' || C == '\n'; });
    return {StepFunctionToken(StepFunctionToken::TokenKind::WHITESPACE),
            Munch.size()};
  }

  static ResultTy munchNumber(const llvm::StringRef &Str) {
    auto Munch = Str.take_while([](auto C) {
      return llvm::isDigit(C) || C == 'e' || C == 'E' || C == '+' || C == '-';
    });
    return {
        StepFunctionToken(StepFunctionToken::TokenKind::NUMBER, Munch.lower()),
        Munch.size()};
  }

  static ResultTy munchIdentifier(const llvm::StringRef &Str) {
    auto Munch =
        Str.take_while([](auto C) { return llvm::isAlnum(C) || C == '_'; });
    return {StepFunctionToken(StepFunctionToken::TokenKind::IDENTIFIER,
                              Munch.str()),
            Munch.size()};
  }

  std::string Cnt;
};

//===----------------------------------------------------------------------===//
//                             StepFunctionParser
//===----------------------------------------------------------------------===//

class StepFunctionParser {
public:
  explicit StepFunctionParser(std::string Cnt,
                              llvm::Optional<unsigned int> Line = llvm::None)
      : TokenList(StepFunctionLexer(std::move(Cnt)).tokenize()), Line(Line) {}

  std::unique_ptr<StepFunction> buildStepFunction() {
    return parseStepFunction();
  }

private:
  [[nodiscard]] const StepFunctionToken &peek() const {
    return TokenList.front();
  }

  [[nodiscard]] StepFunctionToken next() {
    auto Token = TokenList.front();
    TokenList.pop_front();
    return Token;
  }

  bool consume(const StepFunctionToken::TokenKind Kind) {
    if (TokenList.front().getKind() == Kind) {
      TokenList.pop_front();
      return true;
    }
    return false;
  }

  std::unique_ptr<StepFunction> parseStepFunction() {
    llvm::Optional<StepFunction::StepOperator> Op;
    llvm::Optional<std::string> LHS;
    llvm::Optional<double> RHS;
    while (!TokenList.empty()) {
      switch (peek().getKind()) {
      case StepFunctionToken::TokenKind::ERROR:
        assert(peek().getValue().hasValue());
        llvm::errs() << "Lexical error: Unexpected character '"
                     << *peek().getValue() << "'\n";
        return nullptr;
      case StepFunctionToken::TokenKind::END_OF_FILE:
        if (!LHS.hasValue() || !Op.hasValue() || !RHS.hasValue()) {
          llvm::errs() << "Syntax error: Unrecognized expression.\n";
        }
        return std::make_unique<StepFunction>(LHS.getValue(), Op.getValue(),
                                              RHS.getValue());
      case StepFunctionToken::TokenKind::WHITESPACE:
        consume(StepFunctionToken::TokenKind::WHITESPACE);
        continue;
      case StepFunctionToken::TokenKind::PLUS:
        if (Op.hasValue()) {
          llvm::errs() << "Lexical error: Unexpected second operator '"
                       << *peek().getValue() << "'\n";
          return nullptr;
        }
        consume(StepFunctionToken::TokenKind::PLUS);
        Op = StepFunction::StepOperator::ADDITION;
        continue;
      case StepFunctionToken::TokenKind::STAR:
        if (Op.hasValue()) {
          llvm::errs() << "Lexical error: Unexpected second operator '"
                       << *peek().getValue() << "'\n";
          return nullptr;
        }
        consume(StepFunctionToken::TokenKind::STAR);
        Op = StepFunction::StepOperator::MULTIPLICATION;
        continue;
      case StepFunctionToken::TokenKind::IDENTIFIER:
        assert(peek().getValue().hasValue());
        if (Op.hasValue()) {
          llvm::errs() << "Lexical error: Unexpected second identifier '"
                       << *peek().getValue() << "'\n";
          return nullptr;
        }
        LHS = *next().getValue();
        continue;
      case StepFunctionToken::TokenKind::NUMBER:
        assert(peek().getValue().hasValue());
        if (RHS.hasValue()) {
          llvm::errs() << "Lexical error: Unexpected second number '"
                       << *peek().getValue() << "'\n";
          return nullptr;
        }
        double Number;
        llvm::StringRef(*next().getValue()).getAsDouble(Number);
        RHS = Number;
        continue;
      }
    }
    return nullptr;
  }

  StepFunctionLexer::TokenListTy TokenList;
  llvm::Optional<unsigned int> Line;
};

} // namespace vara::feature

#endif // VARA_FEATURE_STEP_FUNCTION_PARSER_H

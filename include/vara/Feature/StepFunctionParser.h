#ifndef VARA_FEATURE_STEPFUNCTIONPARSER_H
#define VARA_FEATURE_STEPFUNCTIONPARSER_H

#include "vara/Feature/StepFunction.h"

#include <llvm/ADT/StringExtras.h>

#include <deque>
#include <optional>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                           StepFunctionToken Class
//===----------------------------------------------------------------------===//

class StepFunctionToken {
public:
  enum class TokenKind {
    IDENTIFIER,
    NUMBER,
    WHITESPACE,
    PLUS,
    STAR,
    CARET,
    END_OF_FILE,
    ERROR
  };

  StepFunctionToken(TokenKind Kind) : Kind(Kind) {}
  StepFunctionToken(TokenKind Kind, const std::string &Value)
      : Kind(Kind), Value(Value) {}
  virtual ~StepFunctionToken() = default;

  [[nodiscard]] TokenKind getKind() const { return Kind; };

  [[nodiscard]] std::optional<const std::string> getValue() const {
    return Value;
  }

private:
  const TokenKind Kind;
  const std::optional<const std::string> Value{std::nullopt};
};

//===----------------------------------------------------------------------===//
//                           StepFunctionLexer Class
//===----------------------------------------------------------------------===//

class StepFunctionLexer {
public:
  using TokenListTy = std::deque<StepFunctionToken>;

  explicit StepFunctionLexer(std::string Cnt) : Cnt(std::move(Cnt)) {}

  [[nodiscard]] TokenListTy tokenize() {
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
    if (('0' <= Str.front() && Str.front() <= '9') || Str.front() == '.') {
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
    case '^':
      return {StepFunctionToken(StepFunctionToken::TokenKind::CARET), 1};
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
    auto Munch =
        Str.take_while([](auto C) { return llvm::isDigit(C) || C == '.'; });
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
//                          StepFunctionParser Class
//===----------------------------------------------------------------------===//

class StepFunctionParser {
public:
  explicit StepFunctionParser(std::string Cnt,
                              std::optional<unsigned int> Line = std::nullopt)
      : TokenList(StepFunctionLexer(std::move(Cnt)).tokenize()), Line(Line) {}

  [[nodiscard]] std::unique_ptr<StepFunction> buildStepFunction() {
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

  std::optional<StepFunction::OperandVariantType> parseOperand() {
    while (!TokenList.empty()) {
      switch (peek().getKind()) {
      case StepFunctionToken::TokenKind::ERROR:
        assert(peek().getValue().has_value());
        llvm::errs() << "Lexical error: Unexpected character '"
                     << *peek().getValue() << "'\n";
        return std::nullopt;
      case StepFunctionToken::TokenKind::END_OF_FILE:
        llvm::errs() << "Syntax error: Unexpected end of expression.\n";
        return std::nullopt;
      case StepFunctionToken::TokenKind::WHITESPACE:
        consume(StepFunctionToken::TokenKind::WHITESPACE);
        continue;
      case StepFunctionToken::TokenKind::PLUS:
        llvm::errs() << "Lexical error: Unexpected operator '+'.\n";
        return std::nullopt;
      case StepFunctionToken::TokenKind::STAR:
        llvm::errs() << "Lexical error: Unexpected operator '*'.\n";
        return std::nullopt;
      case StepFunctionToken::TokenKind::CARET:
        llvm::errs() << "Lexical error: Unexpected operator '^'.\n";
        return std::nullopt;
      case StepFunctionToken::TokenKind::IDENTIFIER:
        assert(peek().getValue().has_value());
        return {*next().getValue()};
      case StepFunctionToken::TokenKind::NUMBER:
        assert(peek().getValue().has_value());
        double Number;
        llvm::StringRef(*next().getValue()).getAsDouble(Number);
        return {Number};
      }
    }
    return std::nullopt;
  }

  std::optional<StepFunction::StepOperation> parseOperator() {
    while (!TokenList.empty()) {
      switch (peek().getKind()) {
      case StepFunctionToken::TokenKind::ERROR:
        assert(peek().getValue().has_value());
        llvm::errs() << "Lexical error: Unexpected character '"
                     << *peek().getValue() << "'\n";
        return std::nullopt;
      case StepFunctionToken::TokenKind::END_OF_FILE:
        llvm::errs() << "Syntax error: Unexpected end of expression.\n";
        return std::nullopt;
      case StepFunctionToken::TokenKind::WHITESPACE:
        consume(StepFunctionToken::TokenKind::WHITESPACE);
        continue;
      case StepFunctionToken::TokenKind::PLUS:
        consume(StepFunctionToken::TokenKind::PLUS);
        return StepFunction::StepOperation::ADDITION;
      case StepFunctionToken::TokenKind::STAR:
        consume(StepFunctionToken::TokenKind::STAR);
        return StepFunction::StepOperation::MULTIPLICATION;
      case StepFunctionToken::TokenKind::CARET:
        consume(StepFunctionToken::TokenKind::CARET);
        return StepFunction::StepOperation::EXPONENTIATION;
      case StepFunctionToken::TokenKind::IDENTIFIER:
        assert(peek().getValue().has_value());
        llvm::errs() << "Syntax error: Unexpected identifier '"
                     << *peek().getValue() << "'\n";
        return std::nullopt;
      case StepFunctionToken::TokenKind::NUMBER:
        assert(peek().getValue().has_value());
        llvm::errs() << "Syntax error: Unexpected number '"
                     << *peek().getValue() << "'\n";
        return std::nullopt;
      }
    }
    return std::nullopt;
  }

  bool parseEOF() {
    while (!TokenList.empty()) {
      switch (peek().getKind()) {
      case StepFunctionToken::TokenKind::ERROR:
        assert(peek().getValue().has_value());
        llvm::errs() << "Lexical error: Unexpected character '"
                     << *peek().getValue() << "'\n";
        return false;
      case StepFunctionToken::TokenKind::END_OF_FILE:
        return true;
      case StepFunctionToken::TokenKind::WHITESPACE:
        consume(StepFunctionToken::TokenKind::WHITESPACE);
        continue;
      case StepFunctionToken::TokenKind::PLUS:
        llvm::errs() << "Lexical error: Unexpected operator '+'.\n";
        return false;
      case StepFunctionToken::TokenKind::STAR:
        llvm::errs() << "Lexical error: Unexpected operator '*'.\n";
        return false;
      case StepFunctionToken::TokenKind::CARET:
        llvm::errs() << "Lexical error: Unexpected operator '^'.\n";
        return false;
      case StepFunctionToken::TokenKind::IDENTIFIER:
        assert(peek().getValue().has_value());
        llvm::errs() << "Syntax error: Unexpected identifier '"
                     << *peek().getValue() << "'\n";
        return false;
      case StepFunctionToken::TokenKind::NUMBER:
        assert(peek().getValue().has_value());
        llvm::errs() << "Syntax error: Unexpected number '"
                     << *peek().getValue() << "'\n";
        return false;
      }
    }
    return false;
  }

  std::unique_ptr<StepFunction> parseStepFunction() {
    auto LHS = parseOperand();
    if (!LHS.has_value()) {
      return nullptr;
    }
    auto Op = parseOperator();
    if (!Op.has_value()) {
      return nullptr;
    }
    auto RHS = parseOperand();
    if (!RHS.has_value()) {
      return nullptr;
    }
    if (!parseEOF()) {
      return nullptr;
    }

    if (std::holds_alternative<std::string>(LHS.value()) &&
        std::holds_alternative<std::string>(RHS.value())) {
      llvm::errs() << "Syntax error: Missing constant.\n";
      return nullptr;
    }
    if (std::holds_alternative<double>(LHS.value()) &&
        std::holds_alternative<double>(RHS.value())) {
      llvm::errs() << "Syntax error: Missing operand.\n";
      return nullptr;
    }

    return std::make_unique<StepFunction>(LHS.value(), Op.value(), RHS.value());
  }

  StepFunctionLexer::TokenListTy TokenList;
  std::optional<unsigned int> Line;
};

} // namespace vara::feature

#endif // VARA_FEATURE_STEPFUNCTIONPARSER_H

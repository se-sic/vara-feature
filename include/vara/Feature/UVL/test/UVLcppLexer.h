
// Generated from UVLcpp.g4 by ANTLR 4.9.2

#pragma once


#include "antlr4-runtime.h"




class  UVLcppLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, OPEN_PAREN = 24, CLOSE_PAREN = 25, 
    OPEN_BRACK = 26, CLOSE_BRACK = 27, OPEN_BRACE = 28, CLOSE_BRACE = 29, 
    OPEN_COMMENT = 30, CLOSE_COMMENT = 31, INDENT = 32, DEDENT = 33, ORGROUP = 34, 
    ALTERNATIVE = 35, OPTIONAL = 36, MANDATORY = 37, CARDINALITY = 38, NOT = 39, 
    AND = 40, OR = 41, EQUIVALENCE = 42, IMPLICATION = 43, EQUAL = 44, LOWER = 45, 
    LOWER_EQUALS = 46, GREATER = 47, GREATER_EQUALS = 48, NOT_EQUALS = 49, 
    DIV = 50, MUL = 51, ADD = 52, SUB = 53, FLOAT = 54, INTEGER = 55, BOOLEAN = 56, 
    BOOLEAN_KEY = 57, COMMA = 58, ID_NOT_STRICT = 59, ID_STRICT = 60, STRING = 61, 
    SKIP_ = 62
  };

  explicit UVLcppLexer(antlr4::CharStream *input);
  ~UVLcppLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

  virtual void action(antlr4::RuleContext *context, size_t ruleIndex, size_t actionIndex) override;
private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.
  void OPEN_PARENAction(antlr4::RuleContext *context, size_t actionIndex);
  void CLOSE_PARENAction(antlr4::RuleContext *context, size_t actionIndex);
  void OPEN_BRACKAction(antlr4::RuleContext *context, size_t actionIndex);
  void CLOSE_BRACKAction(antlr4::RuleContext *context, size_t actionIndex);
  void OPEN_BRACEAction(antlr4::RuleContext *context, size_t actionIndex);
  void CLOSE_BRACEAction(antlr4::RuleContext *context, size_t actionIndex);
  void OPEN_COMMENTAction(antlr4::RuleContext *context, size_t actionIndex);
  void CLOSE_COMMENTAction(antlr4::RuleContext *context, size_t actionIndex);

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};


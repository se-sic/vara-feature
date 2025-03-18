
// Generated from UVLcpp.g4 by ANTLR 4.9.2

#pragma once


#include "antlr4-runtime.h"




class  UVLcppParser : public antlr4::Parser {
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
    SKIP_ = 62, NEWLINE = 63
  };

  enum {
    RuleFeatureModel = 0, RuleIncludes = 1, RuleIncludeLine = 2, RuleNamespace = 3, 
    RuleImports = 4, RuleImportLine = 5, RuleFeatures = 6, RuleGroup = 7, 
    RuleGroupSpec = 8, RuleFeature = 9, RuleFeatureCardinality = 10, RuleAttributes = 11, 
    RuleAttribute = 12, RuleValueAttribute = 13, RuleKey = 14, RuleValue = 15, 
    RuleVector = 16, RuleConstraintAttribute = 17, RuleConstraintList = 18, 
    RuleConstraints = 19, RuleConstraintLine = 20, RuleConstraint = 21, 
    RuleEquation = 22, RuleExpression = 23, RuleAggregateFunction = 24, 
    RuleStringAggregateFunction = 25, RuleNumericAggregateFunction = 26, 
    RuleReference = 27, RuleId = 28, RuleFeatureType = 29, RuleLanguageLevel = 30, 
    RuleMajorLevel = 31, RuleMinorLevel = 32
  };

  explicit UVLcppParser(antlr4::TokenStream *input);
  ~UVLcppParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class FeatureModelContext;
  class IncludesContext;
  class IncludeLineContext;
  class NamespaceContext;
  class ImportsContext;
  class ImportLineContext;
  class FeaturesContext;
  class GroupContext;
  class GroupSpecContext;
  class FeatureContext;
  class FeatureCardinalityContext;
  class AttributesContext;
  class AttributeContext;
  class ValueAttributeContext;
  class KeyContext;
  class ValueContext;
  class VectorContext;
  class ConstraintAttributeContext;
  class ConstraintListContext;
  class ConstraintsContext;
  class ConstraintLineContext;
  class ConstraintContext;
  class EquationContext;
  class ExpressionContext;
  class AggregateFunctionContext;
  class StringAggregateFunctionContext;
  class NumericAggregateFunctionContext;
  class ReferenceContext;
  class IdContext;
  class FeatureTypeContext;
  class LanguageLevelContext;
  class MajorLevelContext;
  class MinorLevelContext; 

  class  FeatureModelContext : public antlr4::ParserRuleContext {
  public:
    FeatureModelContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    NamespaceContext *namespace();
    std::vector<antlr4::tree::TerminalNode *> NEWLINE();
    antlr4::tree::TerminalNode* NEWLINE(size_t i);
    IncludesContext *includes();
    ImportsContext *imports();
    FeaturesContext *features();
    ConstraintsContext *constraints();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FeatureModelContext* featureModel();

  class  IncludesContext : public antlr4::ParserRuleContext {
  public:
    IncludesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NEWLINE();
    antlr4::tree::TerminalNode *INDENT();
    antlr4::tree::TerminalNode *DEDENT();
    std::vector<IncludeLineContext *> includeLine();
    IncludeLineContext* includeLine(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IncludesContext* includes();

  class  IncludeLineContext : public antlr4::ParserRuleContext {
  public:
    IncludeLineContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LanguageLevelContext *languageLevel();
    antlr4::tree::TerminalNode *NEWLINE();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IncludeLineContext* includeLine();

  class  NamespaceContext : public antlr4::ParserRuleContext {
  public:
    NamespaceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ReferenceContext *reference();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NamespaceContext* namespace();

  class  ImportsContext : public antlr4::ParserRuleContext {
  public:
    ImportsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NEWLINE();
    antlr4::tree::TerminalNode *INDENT();
    antlr4::tree::TerminalNode *DEDENT();
    std::vector<ImportLineContext *> importLine();
    ImportLineContext* importLine(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ImportsContext* imports();

  class  ImportLineContext : public antlr4::ParserRuleContext {
  public:
    UVLcppParser::ReferenceContext *ns = nullptr;
    UVLcppParser::ReferenceContext *alias = nullptr;
    ImportLineContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NEWLINE();
    std::vector<ReferenceContext *> reference();
    ReferenceContext* reference(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ImportLineContext* importLine();

  class  FeaturesContext : public antlr4::ParserRuleContext {
  public:
    FeaturesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NEWLINE();
    antlr4::tree::TerminalNode *INDENT();
    FeatureContext *feature();
    antlr4::tree::TerminalNode *DEDENT();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FeaturesContext* features();

  class  GroupContext : public antlr4::ParserRuleContext {
  public:
    GroupContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    GroupContext() = default;
    void copyFrom(GroupContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  AlternativeGroupContext : public GroupContext {
  public:
    AlternativeGroupContext(GroupContext *ctx);

    antlr4::tree::TerminalNode *ALTERNATIVE();
    GroupSpecContext *groupSpec();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  OptionalGroupContext : public GroupContext {
  public:
    OptionalGroupContext(GroupContext *ctx);

    antlr4::tree::TerminalNode *OPTIONAL();
    GroupSpecContext *groupSpec();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MandatoryGroupContext : public GroupContext {
  public:
    MandatoryGroupContext(GroupContext *ctx);

    antlr4::tree::TerminalNode *MANDATORY();
    GroupSpecContext *groupSpec();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  CardinalityGroupContext : public GroupContext {
  public:
    CardinalityGroupContext(GroupContext *ctx);

    antlr4::tree::TerminalNode *CARDINALITY();
    GroupSpecContext *groupSpec();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  OrGroupContext : public GroupContext {
  public:
    OrGroupContext(GroupContext *ctx);

    antlr4::tree::TerminalNode *ORGROUP();
    GroupSpecContext *groupSpec();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  GroupContext* group();

  class  GroupSpecContext : public antlr4::ParserRuleContext {
  public:
    GroupSpecContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NEWLINE();
    antlr4::tree::TerminalNode *INDENT();
    antlr4::tree::TerminalNode *DEDENT();
    std::vector<FeatureContext *> feature();
    FeatureContext* feature(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GroupSpecContext* groupSpec();

  class  FeatureContext : public antlr4::ParserRuleContext {
  public:
    FeatureContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ReferenceContext *reference();
    antlr4::tree::TerminalNode *NEWLINE();
    FeatureTypeContext *featureType();
    FeatureCardinalityContext *featureCardinality();
    AttributesContext *attributes();
    antlr4::tree::TerminalNode *INDENT();
    antlr4::tree::TerminalNode *DEDENT();
    std::vector<GroupContext *> group();
    GroupContext* group(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FeatureContext* feature();

  class  FeatureCardinalityContext : public antlr4::ParserRuleContext {
  public:
    FeatureCardinalityContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CARDINALITY();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FeatureCardinalityContext* featureCardinality();

  class  AttributesContext : public antlr4::ParserRuleContext {
  public:
    AttributesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_BRACE();
    antlr4::tree::TerminalNode *CLOSE_BRACE();
    std::vector<AttributeContext *> attribute();
    AttributeContext* attribute(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttributesContext* attributes();

  class  AttributeContext : public antlr4::ParserRuleContext {
  public:
    AttributeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ValueAttributeContext *valueAttribute();
    ConstraintAttributeContext *constraintAttribute();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AttributeContext* attribute();

  class  ValueAttributeContext : public antlr4::ParserRuleContext {
  public:
    ValueAttributeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    KeyContext *key();
    ValueContext *value();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueAttributeContext* valueAttribute();

  class  KeyContext : public antlr4::ParserRuleContext {
  public:
    KeyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IdContext *id();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  KeyContext* key();

  class  ValueContext : public antlr4::ParserRuleContext {
  public:
    ValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BOOLEAN();
    antlr4::tree::TerminalNode *FLOAT();
    antlr4::tree::TerminalNode *INTEGER();
    antlr4::tree::TerminalNode *STRING();
    AttributesContext *attributes();
    VectorContext *vector();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueContext* value();

  class  VectorContext : public antlr4::ParserRuleContext {
  public:
    VectorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_BRACK();
    antlr4::tree::TerminalNode *CLOSE_BRACK();
    std::vector<ValueContext *> value();
    ValueContext* value(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VectorContext* vector();

  class  ConstraintAttributeContext : public antlr4::ParserRuleContext {
  public:
    ConstraintAttributeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ConstraintAttributeContext() = default;
    void copyFrom(ConstraintAttributeContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ListConstraintAttributeContext : public ConstraintAttributeContext {
  public:
    ListConstraintAttributeContext(ConstraintAttributeContext *ctx);

    ConstraintListContext *constraintList();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SingleConstraintAttributeContext : public ConstraintAttributeContext {
  public:
    SingleConstraintAttributeContext(ConstraintAttributeContext *ctx);

    ConstraintContext *constraint();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ConstraintAttributeContext* constraintAttribute();

  class  ConstraintListContext : public antlr4::ParserRuleContext {
  public:
    ConstraintListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OPEN_BRACK();
    antlr4::tree::TerminalNode *CLOSE_BRACK();
    std::vector<ConstraintContext *> constraint();
    ConstraintContext* constraint(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstraintListContext* constraintList();

  class  ConstraintsContext : public antlr4::ParserRuleContext {
  public:
    ConstraintsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NEWLINE();
    antlr4::tree::TerminalNode *INDENT();
    antlr4::tree::TerminalNode *DEDENT();
    std::vector<ConstraintLineContext *> constraintLine();
    ConstraintLineContext* constraintLine(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstraintsContext* constraints();

  class  ConstraintLineContext : public antlr4::ParserRuleContext {
  public:
    ConstraintLineContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ConstraintContext *constraint();
    antlr4::tree::TerminalNode *NEWLINE();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstraintLineContext* constraintLine();

  class  ConstraintContext : public antlr4::ParserRuleContext {
  public:
    ConstraintContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ConstraintContext() = default;
    void copyFrom(ConstraintContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  OrConstraintContext : public ConstraintContext {
  public:
    OrConstraintContext(ConstraintContext *ctx);

    std::vector<ConstraintContext *> constraint();
    ConstraintContext* constraint(size_t i);
    antlr4::tree::TerminalNode *OR();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  EquationConstraintContext : public ConstraintContext {
  public:
    EquationConstraintContext(ConstraintContext *ctx);

    EquationContext *equation();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LiteralConstraintContext : public ConstraintContext {
  public:
    LiteralConstraintContext(ConstraintContext *ctx);

    ReferenceContext *reference();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ParenthesisConstraintContext : public ConstraintContext {
  public:
    ParenthesisConstraintContext(ConstraintContext *ctx);

    antlr4::tree::TerminalNode *OPEN_PAREN();
    ConstraintContext *constraint();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NotConstraintContext : public ConstraintContext {
  public:
    NotConstraintContext(ConstraintContext *ctx);

    antlr4::tree::TerminalNode *NOT();
    ConstraintContext *constraint();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AndConstraintContext : public ConstraintContext {
  public:
    AndConstraintContext(ConstraintContext *ctx);

    std::vector<ConstraintContext *> constraint();
    ConstraintContext* constraint(size_t i);
    antlr4::tree::TerminalNode *AND();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  EquivalenceConstraintContext : public ConstraintContext {
  public:
    EquivalenceConstraintContext(ConstraintContext *ctx);

    std::vector<ConstraintContext *> constraint();
    ConstraintContext* constraint(size_t i);
    antlr4::tree::TerminalNode *EQUIVALENCE();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ImplicationConstraintContext : public ConstraintContext {
  public:
    ImplicationConstraintContext(ConstraintContext *ctx);

    std::vector<ConstraintContext *> constraint();
    ConstraintContext* constraint(size_t i);
    antlr4::tree::TerminalNode *IMPLICATION();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ConstraintContext* constraint();
  ConstraintContext* constraint(int precedence);
  class  EquationContext : public antlr4::ParserRuleContext {
  public:
    EquationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    EquationContext() = default;
    void copyFrom(EquationContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  EqualEquationContext : public EquationContext {
  public:
    EqualEquationContext(EquationContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *EQUAL();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LowerEquationContext : public EquationContext {
  public:
    LowerEquationContext(EquationContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *LOWER();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LowerEqualsEquationContext : public EquationContext {
  public:
    LowerEqualsEquationContext(EquationContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *LOWER_EQUALS();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  GreaterEqualsEquationContext : public EquationContext {
  public:
    GreaterEqualsEquationContext(EquationContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *GREATER_EQUALS();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  GreaterEquationContext : public EquationContext {
  public:
    GreaterEquationContext(EquationContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *GREATER();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NotEqualsEquationContext : public EquationContext {
  public:
    NotEqualsEquationContext(EquationContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *NOT_EQUALS();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  EquationContext* equation();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ExpressionContext() = default;
    void copyFrom(ExpressionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  BracketExpressionContext : public ExpressionContext {
  public:
    BracketExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *OPEN_PAREN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AggregateFunctionExpressionContext : public ExpressionContext {
  public:
    AggregateFunctionExpressionContext(ExpressionContext *ctx);

    AggregateFunctionContext *aggregateFunction();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FloatLiteralExpressionContext : public ExpressionContext {
  public:
    FloatLiteralExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *FLOAT();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  StringLiteralExpressionContext : public ExpressionContext {
  public:
    StringLiteralExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *STRING();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AddExpressionContext : public ExpressionContext {
  public:
    AddExpressionContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *ADD();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  IntegerLiteralExpressionContext : public ExpressionContext {
  public:
    IntegerLiteralExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *INTEGER();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LiteralExpressionContext : public ExpressionContext {
  public:
    LiteralExpressionContext(ExpressionContext *ctx);

    ReferenceContext *reference();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  DivExpressionContext : public ExpressionContext {
  public:
    DivExpressionContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *DIV();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SubExpressionContext : public ExpressionContext {
  public:
    SubExpressionContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *SUB();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MulExpressionContext : public ExpressionContext {
  public:
    MulExpressionContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *MUL();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  AggregateFunctionContext : public antlr4::ParserRuleContext {
  public:
    AggregateFunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    AggregateFunctionContext() = default;
    void copyFrom(AggregateFunctionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  AvgAggregateFunctionContext : public AggregateFunctionContext {
  public:
    AvgAggregateFunctionContext(AggregateFunctionContext *ctx);

    antlr4::tree::TerminalNode *OPEN_PAREN();
    std::vector<ReferenceContext *> reference();
    ReferenceContext* reference(size_t i);
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    antlr4::tree::TerminalNode *COMMA();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NumericAggregateFunctionExpressionContext : public AggregateFunctionContext {
  public:
    NumericAggregateFunctionExpressionContext(AggregateFunctionContext *ctx);

    NumericAggregateFunctionContext *numericAggregateFunction();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SumAggregateFunctionContext : public AggregateFunctionContext {
  public:
    SumAggregateFunctionContext(AggregateFunctionContext *ctx);

    antlr4::tree::TerminalNode *OPEN_PAREN();
    std::vector<ReferenceContext *> reference();
    ReferenceContext* reference(size_t i);
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    antlr4::tree::TerminalNode *COMMA();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  StringAggregateFunctionExpressionContext : public AggregateFunctionContext {
  public:
    StringAggregateFunctionExpressionContext(AggregateFunctionContext *ctx);

    StringAggregateFunctionContext *stringAggregateFunction();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  AggregateFunctionContext* aggregateFunction();

  class  StringAggregateFunctionContext : public antlr4::ParserRuleContext {
  public:
    StringAggregateFunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    StringAggregateFunctionContext() = default;
    void copyFrom(StringAggregateFunctionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  LengthAggregateFunctionContext : public StringAggregateFunctionContext {
  public:
    LengthAggregateFunctionContext(StringAggregateFunctionContext *ctx);

    antlr4::tree::TerminalNode *OPEN_PAREN();
    ReferenceContext *reference();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  StringAggregateFunctionContext* stringAggregateFunction();

  class  NumericAggregateFunctionContext : public antlr4::ParserRuleContext {
  public:
    NumericAggregateFunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    NumericAggregateFunctionContext() = default;
    void copyFrom(NumericAggregateFunctionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  CeilAggregateFunctionContext : public NumericAggregateFunctionContext {
  public:
    CeilAggregateFunctionContext(NumericAggregateFunctionContext *ctx);

    antlr4::tree::TerminalNode *OPEN_PAREN();
    ReferenceContext *reference();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FloorAggregateFunctionContext : public NumericAggregateFunctionContext {
  public:
    FloorAggregateFunctionContext(NumericAggregateFunctionContext *ctx);

    antlr4::tree::TerminalNode *OPEN_PAREN();
    ReferenceContext *reference();
    antlr4::tree::TerminalNode *CLOSE_PAREN();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  NumericAggregateFunctionContext* numericAggregateFunction();

  class  ReferenceContext : public antlr4::ParserRuleContext {
  public:
    ReferenceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<IdContext *> id();
    IdContext* id(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ReferenceContext* reference();

  class  IdContext : public antlr4::ParserRuleContext {
  public:
    IdContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID_STRICT();
    antlr4::tree::TerminalNode *ID_NOT_STRICT();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IdContext* id();

  class  FeatureTypeContext : public antlr4::ParserRuleContext {
  public:
    FeatureTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BOOLEAN_KEY();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FeatureTypeContext* featureType();

  class  LanguageLevelContext : public antlr4::ParserRuleContext {
  public:
    LanguageLevelContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    MajorLevelContext *majorLevel();
    MinorLevelContext *minorLevel();
    antlr4::tree::TerminalNode *MUL();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LanguageLevelContext* languageLevel();

  class  MajorLevelContext : public antlr4::ParserRuleContext {
  public:
    MajorLevelContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BOOLEAN_KEY();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MajorLevelContext* majorLevel();

  class  MinorLevelContext : public antlr4::ParserRuleContext {
  public:
    MinorLevelContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MinorLevelContext* minorLevel();


  virtual bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;
  bool constraintSempred(ConstraintContext *_localctx, size_t predicateIndex);
  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};



// Generated from UVLcpp.g4 by ANTLR 4.9.2

#pragma once


#include "antlr4-runtime.h"
#include "UVLcppVisitor.h"


/**
 * This class provides an empty implementation of UVLcppVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  UVLcppBaseVisitor : public UVLcppVisitor {
public:

  virtual antlrcpp::Any visitFeatureModel(UVLcppParser::FeatureModelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIncludes(UVLcppParser::IncludesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIncludeLine(UVLcppParser::IncludeLineContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNamespace(UVLcppParser::NamespaceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitImports(UVLcppParser::ImportsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitImportLine(UVLcppParser::ImportLineContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFeatures(UVLcppParser::FeaturesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOrGroup(UVLcppParser::OrGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAlternativeGroup(UVLcppParser::AlternativeGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOptionalGroup(UVLcppParser::OptionalGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMandatoryGroup(UVLcppParser::MandatoryGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCardinalityGroup(UVLcppParser::CardinalityGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGroupSpec(UVLcppParser::GroupSpecContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFeature(UVLcppParser::FeatureContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFeatureCardinality(UVLcppParser::FeatureCardinalityContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAttributes(UVLcppParser::AttributesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAttribute(UVLcppParser::AttributeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValueAttribute(UVLcppParser::ValueAttributeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitKey(UVLcppParser::KeyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValue(UVLcppParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVector(UVLcppParser::VectorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSingleConstraintAttribute(UVLcppParser::SingleConstraintAttributeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitListConstraintAttribute(UVLcppParser::ListConstraintAttributeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstraintList(UVLcppParser::ConstraintListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstraints(UVLcppParser::ConstraintsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstraintLine(UVLcppParser::ConstraintLineContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOrConstraint(UVLcppParser::OrConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEquationConstraint(UVLcppParser::EquationConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLiteralConstraint(UVLcppParser::LiteralConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitParenthesisConstraint(UVLcppParser::ParenthesisConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNotConstraint(UVLcppParser::NotConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAndConstraint(UVLcppParser::AndConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEquivalenceConstraint(UVLcppParser::EquivalenceConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitImplicationConstraint(UVLcppParser::ImplicationConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEqualEquation(UVLcppParser::EqualEquationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLowerEquation(UVLcppParser::LowerEquationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGreaterEquation(UVLcppParser::GreaterEquationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLowerEqualsEquation(UVLcppParser::LowerEqualsEquationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGreaterEqualsEquation(UVLcppParser::GreaterEqualsEquationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNotEqualsEquation(UVLcppParser::NotEqualsEquationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBracketExpression(UVLcppParser::BracketExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAggregateFunctionExpression(UVLcppParser::AggregateFunctionExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFloatLiteralExpression(UVLcppParser::FloatLiteralExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStringLiteralExpression(UVLcppParser::StringLiteralExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAddExpression(UVLcppParser::AddExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIntegerLiteralExpression(UVLcppParser::IntegerLiteralExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLiteralExpression(UVLcppParser::LiteralExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDivExpression(UVLcppParser::DivExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSubExpression(UVLcppParser::SubExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMulExpression(UVLcppParser::MulExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSumAggregateFunction(UVLcppParser::SumAggregateFunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAvgAggregateFunction(UVLcppParser::AvgAggregateFunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStringAggregateFunctionExpression(UVLcppParser::StringAggregateFunctionExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNumericAggregateFunctionExpression(UVLcppParser::NumericAggregateFunctionExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLengthAggregateFunction(UVLcppParser::LengthAggregateFunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFloorAggregateFunction(UVLcppParser::FloorAggregateFunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCeilAggregateFunction(UVLcppParser::CeilAggregateFunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitReference(UVLcppParser::ReferenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitId(UVLcppParser::IdContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFeatureType(UVLcppParser::FeatureTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLanguageLevel(UVLcppParser::LanguageLevelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMajorLevel(UVLcppParser::MajorLevelContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMinorLevel(UVLcppParser::MinorLevelContext *ctx) override {
    return visitChildren(ctx);
  }


};


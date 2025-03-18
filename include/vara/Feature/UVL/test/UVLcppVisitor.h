
// Generated from UVLcpp.g4 by ANTLR 4.9.2

#pragma once


#include "antlr4-runtime.h"
#include "UVLcppParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by UVLcppParser.
 */
class  UVLcppVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by UVLcppParser.
   */
    virtual antlrcpp::Any visitFeatureModel(UVLcppParser::FeatureModelContext *context) = 0;

    virtual antlrcpp::Any visitIncludes(UVLcppParser::IncludesContext *context) = 0;

    virtual antlrcpp::Any visitIncludeLine(UVLcppParser::IncludeLineContext *context) = 0;

    virtual antlrcpp::Any visitNamespace(UVLcppParser::NamespaceContext *context) = 0;

    virtual antlrcpp::Any visitImports(UVLcppParser::ImportsContext *context) = 0;

    virtual antlrcpp::Any visitImportLine(UVLcppParser::ImportLineContext *context) = 0;

    virtual antlrcpp::Any visitFeatures(UVLcppParser::FeaturesContext *context) = 0;

    virtual antlrcpp::Any visitOrGroup(UVLcppParser::OrGroupContext *context) = 0;

    virtual antlrcpp::Any visitAlternativeGroup(UVLcppParser::AlternativeGroupContext *context) = 0;

    virtual antlrcpp::Any visitOptionalGroup(UVLcppParser::OptionalGroupContext *context) = 0;

    virtual antlrcpp::Any visitMandatoryGroup(UVLcppParser::MandatoryGroupContext *context) = 0;

    virtual antlrcpp::Any visitCardinalityGroup(UVLcppParser::CardinalityGroupContext *context) = 0;

    virtual antlrcpp::Any visitGroupSpec(UVLcppParser::GroupSpecContext *context) = 0;

    virtual antlrcpp::Any visitFeature(UVLcppParser::FeatureContext *context) = 0;

    virtual antlrcpp::Any visitFeatureCardinality(UVLcppParser::FeatureCardinalityContext *context) = 0;

    virtual antlrcpp::Any visitAttributes(UVLcppParser::AttributesContext *context) = 0;

    virtual antlrcpp::Any visitAttribute(UVLcppParser::AttributeContext *context) = 0;

    virtual antlrcpp::Any visitValueAttribute(UVLcppParser::ValueAttributeContext *context) = 0;

    virtual antlrcpp::Any visitKey(UVLcppParser::KeyContext *context) = 0;

    virtual antlrcpp::Any visitValue(UVLcppParser::ValueContext *context) = 0;

    virtual antlrcpp::Any visitVector(UVLcppParser::VectorContext *context) = 0;

    virtual antlrcpp::Any visitSingleConstraintAttribute(UVLcppParser::SingleConstraintAttributeContext *context) = 0;

    virtual antlrcpp::Any visitListConstraintAttribute(UVLcppParser::ListConstraintAttributeContext *context) = 0;

    virtual antlrcpp::Any visitConstraintList(UVLcppParser::ConstraintListContext *context) = 0;

    virtual antlrcpp::Any visitConstraints(UVLcppParser::ConstraintsContext *context) = 0;

    virtual antlrcpp::Any visitConstraintLine(UVLcppParser::ConstraintLineContext *context) = 0;

    virtual antlrcpp::Any visitOrConstraint(UVLcppParser::OrConstraintContext *context) = 0;

    virtual antlrcpp::Any visitEquationConstraint(UVLcppParser::EquationConstraintContext *context) = 0;

    virtual antlrcpp::Any visitLiteralConstraint(UVLcppParser::LiteralConstraintContext *context) = 0;

    virtual antlrcpp::Any visitParenthesisConstraint(UVLcppParser::ParenthesisConstraintContext *context) = 0;

    virtual antlrcpp::Any visitNotConstraint(UVLcppParser::NotConstraintContext *context) = 0;

    virtual antlrcpp::Any visitAndConstraint(UVLcppParser::AndConstraintContext *context) = 0;

    virtual antlrcpp::Any visitEquivalenceConstraint(UVLcppParser::EquivalenceConstraintContext *context) = 0;

    virtual antlrcpp::Any visitImplicationConstraint(UVLcppParser::ImplicationConstraintContext *context) = 0;

    virtual antlrcpp::Any visitEqualEquation(UVLcppParser::EqualEquationContext *context) = 0;

    virtual antlrcpp::Any visitLowerEquation(UVLcppParser::LowerEquationContext *context) = 0;

    virtual antlrcpp::Any visitGreaterEquation(UVLcppParser::GreaterEquationContext *context) = 0;

    virtual antlrcpp::Any visitLowerEqualsEquation(UVLcppParser::LowerEqualsEquationContext *context) = 0;

    virtual antlrcpp::Any visitGreaterEqualsEquation(UVLcppParser::GreaterEqualsEquationContext *context) = 0;

    virtual antlrcpp::Any visitNotEqualsEquation(UVLcppParser::NotEqualsEquationContext *context) = 0;

    virtual antlrcpp::Any visitBracketExpression(UVLcppParser::BracketExpressionContext *context) = 0;

    virtual antlrcpp::Any visitAggregateFunctionExpression(UVLcppParser::AggregateFunctionExpressionContext *context) = 0;

    virtual antlrcpp::Any visitFloatLiteralExpression(UVLcppParser::FloatLiteralExpressionContext *context) = 0;

    virtual antlrcpp::Any visitStringLiteralExpression(UVLcppParser::StringLiteralExpressionContext *context) = 0;

    virtual antlrcpp::Any visitAddExpression(UVLcppParser::AddExpressionContext *context) = 0;

    virtual antlrcpp::Any visitIntegerLiteralExpression(UVLcppParser::IntegerLiteralExpressionContext *context) = 0;

    virtual antlrcpp::Any visitLiteralExpression(UVLcppParser::LiteralExpressionContext *context) = 0;

    virtual antlrcpp::Any visitDivExpression(UVLcppParser::DivExpressionContext *context) = 0;

    virtual antlrcpp::Any visitSubExpression(UVLcppParser::SubExpressionContext *context) = 0;

    virtual antlrcpp::Any visitMulExpression(UVLcppParser::MulExpressionContext *context) = 0;

    virtual antlrcpp::Any visitSumAggregateFunction(UVLcppParser::SumAggregateFunctionContext *context) = 0;

    virtual antlrcpp::Any visitAvgAggregateFunction(UVLcppParser::AvgAggregateFunctionContext *context) = 0;

    virtual antlrcpp::Any visitStringAggregateFunctionExpression(UVLcppParser::StringAggregateFunctionExpressionContext *context) = 0;

    virtual antlrcpp::Any visitNumericAggregateFunctionExpression(UVLcppParser::NumericAggregateFunctionExpressionContext *context) = 0;

    virtual antlrcpp::Any visitLengthAggregateFunction(UVLcppParser::LengthAggregateFunctionContext *context) = 0;

    virtual antlrcpp::Any visitFloorAggregateFunction(UVLcppParser::FloorAggregateFunctionContext *context) = 0;

    virtual antlrcpp::Any visitCeilAggregateFunction(UVLcppParser::CeilAggregateFunctionContext *context) = 0;

    virtual antlrcpp::Any visitReference(UVLcppParser::ReferenceContext *context) = 0;

    virtual antlrcpp::Any visitId(UVLcppParser::IdContext *context) = 0;

    virtual antlrcpp::Any visitFeatureType(UVLcppParser::FeatureTypeContext *context) = 0;

    virtual antlrcpp::Any visitLanguageLevel(UVLcppParser::LanguageLevelContext *context) = 0;

    virtual antlrcpp::Any visitMajorLevel(UVLcppParser::MajorLevelContext *context) = 0;

    virtual antlrcpp::Any visitMinorLevel(UVLcppParser::MinorLevelContext *context) = 0;


};


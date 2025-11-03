// handled by cmakelists of upper directory
// #include <vara/Feature/FeatureModel.h>
// #include "vara/Feature/Feature.h"

#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelBuilder.h"
#include "vara/Feature/FeatureModelParser.h"

#ifdef BUILD_UVL_PARSER

#include "antlr4-runtime.h"
#include "antlr4cpp_generated_src/UVLcppLexer/UVLcppLexer.h"
#include "antlr4cpp_generated_src/UVLcppParser/UVLcppParser.h"
#include "antlr4cpp_generated_src/UVLcppParser/UVLcppParserBaseVisitor.h"
#include "vara/Solver/Error.h"

#include <iostream>
#include <string>

using namespace antlr4;
/** This visitor class visits all features in the uvl file and stores them
 * according to hierarchy and type\
 *
 **/
namespace vara::feature {
class FeatureVisitor : public antlrcpp::UVLcppParserBaseVisitor {
public:
  std::string CurrentParent;
  std::string PreviousParent;
  FeatureModelBuilder FMB;
  std::unique_ptr<FeatureModel> FM;
  std::stack<std::string> ParentStack; // To track depth of parents as we move
  // further down in the tree
  std::stack<std::unique_ptr<Constraint>>
      ConstraintStack; // To track and compile constraints as we traverse
  // through them
  bool Optional = false;

  // Feature addition impl
  antlrcpp::Any
  visitFeature(antlrcpp::UVLcppParser::FeatureContext *Ctx) override {
    if (Ctx->featureCardinality()) {
      std::cout << "Cardinality based features are not supported yet." << '\n';
      return Result<solver::SolverErrorCode>(solver::NOT_SUPPORTED);
    }
    if (Ctx->featureType()) {
      auto FType = Ctx->featureType()->getText();
      if (FType == "Integer") {
        FMB.makeFeature<NumericFeature>(Ctx->reference()->getText(),
                                        NumericFeature::ValueListType{},
                                        Optional);
      } else if (FType == "Real" || FType == "String") {
        return Result<solver::SolverErrorCode>(solver::NOT_SUPPORTED);
      }
    } else {
      FMB.makeFeature<BinaryFeature>(Ctx->reference()->getText(), Optional);
    }

    ParentStack.push(Ctx->reference()->getText());
    for (auto *GroupCtx : Ctx->group()) {
      visit(GroupCtx);
    }
    ParentStack.pop();
    return vara::Result<bool>(true);
  }

  antlrcpp::Any
  addRelationShipGroup(antlrcpp::UVLcppParser::GroupSpecContext *Ctx,
                       Relationship::RelationshipKind RK) {
    auto PreOptional = Optional;
    Optional = false;
    visitGroupSpec(Ctx);
    for (auto *F : Ctx->feature()) {
      FMB.addEdge(ParentStack.top(), F->reference()->getText());
    }
    FMB.emplaceRelationship(RK, ParentStack.top());
    Optional = PreOptional;
    return vara::Result<bool>(true);
  }

  antlrcpp::Any
  visitOrGroup(antlrcpp::UVLcppParser::OrGroupContext *Ctx) override {
    return addRelationShipGroup(Ctx->groupSpec(),
                                Relationship::RelationshipKind::RK_OR);
  }

  antlrcpp::Any visitMandatoryGroup(
      antlrcpp::UVLcppParser::MandatoryGroupContext *Ctx) override {
    auto PreOptional = Optional;
    Optional = false;
    visitGroupSpec(Ctx->groupSpec());
    for (auto *F : Ctx->groupSpec()->feature()) {
      FMB.addEdge(ParentStack.top(), F->reference()->getText());
    }
    Optional = PreOptional;
    return vara::Result<bool>(true);
  }

  antlrcpp::Any visitAlternativeGroup(
      antlrcpp::UVLcppParser::AlternativeGroupContext *Ctx) override {
    return addRelationShipGroup(Ctx->groupSpec(),
                                Relationship::RelationshipKind::RK_ALTERNATIVE);
  }

  antlrcpp::Any visitOptionalGroup(
      antlrcpp::UVLcppParser::OptionalGroupContext *Ctx) override {
    auto PreOptional = Optional;
    Optional = true;
    visitGroupSpec(Ctx->groupSpec());
    for (auto *F : Ctx->groupSpec()->feature()) {
      FMB.addEdge(ParentStack.top(), F->reference()->getText());
    }
    Optional = PreOptional;
    return vara::Result<bool>(true);
  }

  antlrcpp::Any visitCardinalityGroup(
      antlrcpp::UVLcppParser::CardinalityGroupContext *Ctx) override {
    return vara::Result<vara::solver::SolverErrorCode>(solver::NOT_SUPPORTED);
  }

  antlrcpp::Any
  visitConstraints(antlrcpp::UVLcppParser::ConstraintsContext *Ctx) override {
    for (auto &Constraint : Ctx->constraintLine()) {
      visitChildren(Constraint);
      auto FConstraint = std::move(ConstraintStack.top());
      ConstraintStack.pop();
      // If the constraint is of kind less, greater, equal, not equal, add it as
      // a numeric constraint
      if (FConstraint->getKind() == Constraint::ConstraintKind::CK_LESS ||
          FConstraint->getKind() == Constraint::ConstraintKind::CK_GREATER ||
          FConstraint->getKind() == Constraint::ConstraintKind::CK_LESS_EQUAL ||
          FConstraint->getKind() ==
              Constraint::ConstraintKind::CK_GREATER_EQUAL ||
          FConstraint->getKind() == Constraint::ConstraintKind::CK_EQUAL ||
          FConstraint->getKind() == Constraint::ConstraintKind::CK_NOT_EQUAL) {
        FMB.addConstraint(std::make_unique<FeatureModel::NonBooleanConstraint>(
            std::move(FConstraint)));
      } else {
        FMB.addConstraint(std::make_unique<FeatureModel::BooleanConstraint>(
            std::move(FConstraint)));
      }
    }
    return Result<bool>(true);
  }

  /**
   *Template function to parse binary constraint operations
   */
  template <typename T>
  Result<bool> parseConstraintOperation(ParserRuleContext *LeftCtx,
                                        ParserRuleContext *RightCtx) {
    visit(LeftCtx);
    auto Left = std::move(ConstraintStack.top());
    ConstraintStack.pop();
    visit(RightCtx);
    auto Right = std::move(ConstraintStack.top());
    ConstraintStack.pop();
    auto Constraint = std::make_unique<T>(std::move(Left), std::move(Right));
    ConstraintStack.push(std::move(Constraint));
    return true;
  }

  antlrcpp::Any visitLiteralConstraint(
      antlrcpp::UVLcppParser::LiteralConstraintContext *Ctx) override {
    auto Feature =
        std::make_unique<feature::Feature>(Ctx->reference()->getText());
    auto LiteralConstraint =
        std::make_unique<PrimaryFeatureConstraint>(std::move(Feature));

    ConstraintStack.push(std::move(LiteralConstraint));
    return Result<bool>(true);
  }

  antlrcpp::Any visitNotConstraint(
      antlrcpp::UVLcppParser::NotConstraintContext *Ctx) override {
    visit(Ctx->constraint());
    auto Inner = std::move(ConstraintStack.top());
    ConstraintStack.pop();

    ConstraintStack.push(std::make_unique<NotConstraint>(std::move(Inner)));
    return Result<bool>(true);
    ;
  }

  antlrcpp::Any visitAndConstraint(
      antlrcpp::UVLcppParser::AndConstraintContext *Ctx) override {
    return parseConstraintOperation<feature::AndConstraint>(Ctx->constraint(0),
                                                            Ctx->constraint(1));
  }

  antlrcpp::Any
  visitOrConstraint(antlrcpp::UVLcppParser::OrConstraintContext *Ctx) override {
    return parseConstraintOperation<feature::OrConstraint>(Ctx->constraint(0),
                                                           Ctx->constraint(1));
  }

  antlrcpp::Any visitImplicationConstraint(
      antlrcpp::UVLcppParser::ImplicationConstraintContext *Ctx) override {
    return parseConstraintOperation<feature::ImpliesConstraint>(
        Ctx->constraint(0), Ctx->constraint(1));
  }

  std::any
  visitFeatures(antlrcpp::UVLcppParser::FeaturesContext *Ctx) override {
    FMB.makeRoot(Ctx->feature()->reference()->getText());
    ParentStack.push(Ctx->feature()->reference()->getText());
    for (auto *F : Ctx->feature()->group()) {
      visit(F);
    }
    return vara::Result<bool>(true);
  }

  std::any visitDivExpression(
      antlrcpp::UVLcppParser::DivExpressionContext *Ctx) override {
    return parseConstraintOperation<DivisionConstraint>(
        Ctx->multiplicativeExpression(), Ctx->primaryExpression());
  }

  std::any visitMulExpression(
      antlrcpp::UVLcppParser::MulExpressionContext *Ctx) override {
    return parseConstraintOperation<MultiplicationConstraint>(
        Ctx->multiplicativeExpression(), Ctx->primaryExpression());
  }

  std::any visitIntegerLiteralExpression(
      antlrcpp::UVLcppParser::IntegerLiteralExpressionContext *Ctx) override {
    ConstraintStack.push(
        std::make_unique<PrimaryIntegerConstraint>(std::stoi(Ctx->getText())));
    return vara::Result<bool>(true);
  }

  std::any visitLiteralExpression(
      antlrcpp::UVLcppParser::LiteralExpressionContext *Ctx) override {
    ConstraintStack.push(std::make_unique<PrimaryFeatureConstraint>(
        std::make_unique<Feature>(Ctx->getText())));
    return vara::Result<bool>(true);
  }

  std::any visitAddExpression(
      antlrcpp::UVLcppParser::AddExpressionContext *Ctx) override {
    return parseConstraintOperation<AdditionConstraint>(
        Ctx->additiveExpression(), Ctx->multiplicativeExpression());
  }

  std::any visitSubExpression(
      antlrcpp::UVLcppParser::SubExpressionContext *Ctx) override {
    return parseConstraintOperation<SubtractionConstraint>(
        Ctx->additiveExpression(), Ctx->multiplicativeExpression());
  }

  std::any visitEqualEquation(
      antlrcpp::UVLcppParser::EqualEquationContext *Ctx) override {
    return parseConstraintOperation<EqualConstraint>(Ctx->expression(0),
                                                     Ctx->expression(1));
  }

  std::any visitLowerEquation(
      antlrcpp::UVLcppParser::LowerEquationContext *Ctx) override {
    return parseConstraintOperation<LessConstraint>(Ctx->expression(0),
                                                    Ctx->expression(1));
  }

  std::any visitLowerEqualsEquation(
      antlrcpp::UVLcppParser::LowerEqualsEquationContext *Ctx) override {
    return parseConstraintOperation<LessEqualConstraint>(Ctx->expression(0),
                                                         Ctx->expression(1));
  }

  std::any visitGreaterEqualsEquation(
      antlrcpp::UVLcppParser::GreaterEqualsEquationContext *Ctx) override {
    return parseConstraintOperation<GreaterEqualConstraint>(Ctx->expression(0),
                                                            Ctx->expression(1));
  }

  std::any visitGreaterEquation(
      antlrcpp::UVLcppParser::GreaterEquationContext *Ctx) override {
    return parseConstraintOperation<GreaterConstraint>(Ctx->expression(0),
                                                       Ctx->expression(1));
  }

  std::any visitNotEqualsEquation(
      antlrcpp::UVLcppParser::NotEqualsEquationContext *Ctx) override {
    return parseConstraintOperation<NotEqualConstraint>(Ctx->expression(0),
                                                        Ctx->expression(1));
  }

  antlrcpp::Any
  visitFeatureModel(antlrcpp::UVLcppParser::FeatureModelContext *Ctx) override {
    visitChildren(Ctx);
    return true;
  }
};

std::unique_ptr<FeatureModel> FeatureModelUvlParser::buildFeatureModel() {
  ANTLRInputStream Input(Uvl);
  antlrcpp::UVLcppLexer Lexer(&Input);
  CommonTokenStream Tokens(&Lexer);

  Tokens.fill();

  antlrcpp::UVLcppParser Parser(&Tokens);
  tree::ParseTree *Tree = Parser.featureModel();

  FeatureVisitor Visitor;
  Visitor.visit(Tree);
  return Visitor.FMB.buildFeatureModel();
}

Result<FTErrorCode> FeatureModelUvlParser::verifyFeatureModel() { return Ok(); }

FeatureModelUvlParser::~FeatureModelUvlParser() = default;

} // namespace vara::feature
#endif // BUILD_UVL_PARSER

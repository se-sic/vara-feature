
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
  vara::feature::FeatureModelBuilder FMB;
  std::unique_ptr<vara::feature::FeatureModel> FM;
  std::stack<std::string> ParentStack; // To track depth of parents as we move
  // further down in the tree
  std::stack<std::unique_ptr<vara::feature::Constraint>>
      ConstraintStack; // To track and compile constraints as we traverse
  // through them
  bool Optional = false;

  // Feature addition impl
  antlrcpp::Any
  visitFeature(antlrcpp::UVLcppParser::FeatureContext *Ctx) override {
    if (Ctx->featureCardinality()) {
      std::cout << "Cardinality based features are not supported yet." << '\n';
      return vara::Result<vara::solver::SolverErrorCode>(
          vara::solver::NOT_SUPPORTED);
    }
    if (Ctx->featureType()) {
      auto FType = Ctx->featureType()->getText();
      if (FType == "Integer") {
        FMB.makeFeature<vara::feature::NumericFeature>(
            Ctx->reference()->getText(), NumericFeature::ValueListType{},
            Optional);
      } else if (FType == "Real" || FType == "String") {
        return vara::Result<vara::solver::SolverErrorCode>(
            vara::solver::NOT_SUPPORTED);
      }
    } else {
      FMB.makeFeature<vara::feature::BinaryFeature>(Ctx->reference()->getText(),
                                                    Optional);
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
    return vara::Result<vara::solver::SolverErrorCode>(
        vara::solver::NOT_SUPPORTED);
  }

  antlrcpp::Any
  visitConstraints(antlrcpp::UVLcppParser::ConstraintsContext *Ctx) override {
    for (auto &Constraint : Ctx->constraintLine()) {
      visitChildren(Constraint);
      FMB.addConstraint(
          std::make_unique<vara::feature::FeatureModel::BooleanConstraint>(
              std::move(ConstraintStack.top())));
      ConstraintStack.pop();
    }
    return nullptr;
  }

  antlrcpp::Any visitLiteralConstraint(
      antlrcpp::UVLcppParser::LiteralConstraintContext *Ctx) override {
    auto Feature =
        std::make_unique<vara::feature::Feature>(Ctx->reference()->getText());
    auto LiteralConstraint =
        std::make_unique<vara::feature::PrimaryFeatureConstraint>(
            std::move(Feature));

    ConstraintStack.push(std::move(LiteralConstraint));
    return nullptr;
  }

  antlrcpp::Any visitNotConstraint(
      antlrcpp::UVLcppParser::NotConstraintContext *Ctx) override {
    visit(Ctx->constraint());
    auto Inner = std::move(ConstraintStack.top());
    ConstraintStack.pop();

    ConstraintStack.push(
        std::make_unique<vara::feature::NotConstraint>(std::move(Inner)));
    return nullptr;
  }

  antlrcpp::Any visitAndConstraint(
      antlrcpp::UVLcppParser::AndConstraintContext *Ctx) override {
    visit(Ctx->constraint(0));
    visit(Ctx->constraint(1));

    auto Right = std::move(ConstraintStack.top());
    ConstraintStack.pop();
    auto Left = std::move(ConstraintStack.top());
    ConstraintStack.pop();

    auto AndConstraint = std::make_unique<vara::feature::AndConstraint>(
        std::move(Left), std::move(Right));
    ConstraintStack.push(std::move(AndConstraint));
    return nullptr;
  }

  antlrcpp::Any
  visitOrConstraint(antlrcpp::UVLcppParser::OrConstraintContext *Ctx) override {
    visit(Ctx->constraint(0));
    visit(Ctx->constraint(1));

    auto Right = std::move(ConstraintStack.top());
    ConstraintStack.pop();
    auto Left = std::move(ConstraintStack.top());
    ConstraintStack.pop();

    auto AndConstraint = std::make_unique<vara::feature::OrConstraint>(
        std::move(Left), std::move(Right));
    ConstraintStack.push(std::move(AndConstraint));
    return nullptr;
  }

  antlrcpp::Any visitImplicationConstraint(
      antlrcpp::UVLcppParser::ImplicationConstraintContext *Ctx) override {
    visit(Ctx->constraint(0));
    auto Left = std::move(ConstraintStack.top());
    ConstraintStack.pop();
    visit(Ctx->constraint(1));
    auto Right = std::move(ConstraintStack.top());
    ConstraintStack.pop();

    auto ImplConstraint = std::make_unique<vara::feature::ImpliesConstraint>(
        std::move(Left), std::move(Right));
    ConstraintStack.push(std::move(ImplConstraint));
    return nullptr;
  }

  std::any
  visitFeatures(antlrcpp::UVLcppParser::FeaturesContext *Context) override {
    FMB.makeRoot(Context->feature()->reference()->getText());
    ParentStack.push(Context->feature()->reference()->getText());
    for (auto *F : Context->feature()->group()) {
      visit(F);
    }
    return vara::Result<bool>(true);
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
} // namespace vara::feature
#endif // BUILD_UVL_PARSER


// handled by cmakelists of upper directory
// #include <vara/Feature/FeatureModel.h>
// #include "vara/Feature/Feature.h"

#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"

#include "vara/Feature/FeatureModelBuilder.h"

#include <unistd.h>
#ifdef ANTLR_AVAILABLE
// this block

#include "antlr4-runtime.h"
#include "antlr4cpp_generated_src/UVLcpp/UVLcppBaseVisitor.h" +
#include "antlr4cpp_generated_src/UVLcpp/UVLcppLexer.h"
#include <fstream>
#include <iostream>
#include <string>

#include "antlr4cpp_generated_src/UVLcpp/UVLcppParser.h"

using namespace antlr4;

/** This visitor class visits all features in the uvl file and stores them
 * according to hierarchy and type\
 *
 **/
class FeatureVisitor : public antlrcpp::UVLcppBaseVisitor {
public:
  std::string current_parent;
  std::string previous_parent;
  vara::feature::FeatureModelBuilder FMB;
  std::unique_ptr<vara::feature::FeatureModel> FM;
  std::stack<std::string> parent_stack; // To track depth of parents as we move
                                        // further down in the tree
  std::stack<std::unique_ptr<vara::feature::Constraint>>
      constraint_stack; // To track and compile constraints as we traverse
                        // through them

  // Feature addition impl
  antlrcpp::Any
  visitFeature(antlrcpp::UVLcppParser::FeatureContext *ctx) override {
    std::cout << "visitFeature" << std::endl;
    previous_parent = current_parent;
    current_parent = ctx->reference()->getText();

    parent_stack.push(current_parent);
    for (auto groupCtx :
         ctx->getRuleContexts<antlrcpp::UVLcppParser::GroupContext>()) {
      visit(groupCtx);
    }
    parent_stack.pop();
    return true;
  }

  antlrcpp::Any
  visitOrGroup(antlrcpp::UVLcppParser::OrGroupContext *ctx) override {
    for (auto f : ctx->groupSpec()->feature()) {
      FMB.makeFeature<vara::feature::BinaryFeature>(f->reference()->getText(),
                                                    true);
      FMB.addEdge(parent_stack.top(),
                  ctx->groupSpec()->feature().front()->reference()->getText());
    }
    return visitGroupSpec(ctx->groupSpec());
  }

  antlrcpp::Any visitMandatoryGroup(
      antlrcpp::UVLcppParser::MandatoryGroupContext *ctx) override {
    for (auto f : ctx->groupSpec()->feature()) {
      FMB.makeFeature<vara::feature::BinaryFeature>(f->reference()->getText(),
                                                    false);
      FMB.addEdge(parent_stack.top(), f->reference()->getText());
    }
    return visitGroupSpec(ctx->groupSpec());
  }

  antlrcpp::Any visitAlternativeGroup(
      antlrcpp::UVLcppParser::AlternativeGroupContext *ctx) override {
    FMB.makeFeature<vara::feature::BinaryFeature>(
        ctx->groupSpec()->feature().front()->reference()->getText(), true);
    FMB.addEdge(parent_stack.top(),
                ctx->groupSpec()->feature().front()->reference()->getText());
    return visitGroupSpec(ctx->groupSpec());
  }

  antlrcpp::Any visitOptionalGroup(
      antlrcpp::UVLcppParser::OptionalGroupContext *ctx) override {
    for (auto f : ctx->groupSpec()->feature()) {
      FMB.makeFeature<vara::feature::BinaryFeature>(f->reference()->getText(),
                                                    true);
      FMB.addEdge(parent_stack.top(), f->reference()->getText());
    }
    return visitGroupSpec(ctx->groupSpec());
  }

  antlrcpp::Any visitCardinalityGroup(
      antlrcpp::UVLcppParser::CardinalityGroupContext *ctx) override {
    return visitGroupSpec(ctx->groupSpec());
  }

  antlrcpp::Any
  visitConstraints(antlrcpp::UVLcppParser::ConstraintsContext *ctx) override {
    for (auto &constraint : ctx->constraintLine()) {
      visitChildren(constraint);
      FMB.addConstraint(
          std::make_unique<vara::feature::FeatureModel::BooleanConstraint>(
              std::move(constraint_stack.top())));
      constraint_stack.pop();
    }
    return nullptr;
  }

  antlrcpp::Any visitLiteralConstraint(
      antlrcpp::UVLcppParser::LiteralConstraintContext *ctx) override {
    auto feature =
        std::make_unique<vara::feature::Feature>(ctx->reference()->getText());
    auto literal_constraint =
        std::make_unique<vara::feature::PrimaryFeatureConstraint>(
            std::move(feature));

    constraint_stack.push(std::move(literal_constraint));
    return nullptr;
  }

  antlrcpp::Any visitNotConstraint(
      antlrcpp::UVLcppParser::NotConstraintContext *ctx) override {
    visit(ctx->constraint());
    auto inner = std::move(constraint_stack.top());
    constraint_stack.pop();

    constraint_stack.push(
        std::make_unique<vara::feature::NotConstraint>(std::move(inner)));
    return nullptr;
  }

  antlrcpp::Any visitAndConstraint(
      antlrcpp::UVLcppParser::AndConstraintContext *ctx) override {
    visit(ctx->constraint(0));
    visit(ctx->constraint(1));

    auto right = std::move(constraint_stack.top());
    constraint_stack.pop();
    auto left = std::move(constraint_stack.top());
    constraint_stack.pop();

    auto and_constraint = std::make_unique<vara::feature::AndConstraint>(
        std::move(left), std::move(right));
    constraint_stack.push(std::move(and_constraint));
    return nullptr;
  }

  antlrcpp::Any
  visitOrConstraint(antlrcpp::UVLcppParser::OrConstraintContext *ctx) override {
    visit(ctx->constraint(0));
    visit(ctx->constraint(1));

    auto right = std::move(constraint_stack.top());
    constraint_stack.pop();
    auto left = std::move(constraint_stack.top());
    constraint_stack.pop();

    auto and_constraint = std::make_unique<vara::feature::OrConstraint>(
        std::move(left), std::move(right));
    constraint_stack.push(std::move(and_constraint));
    return nullptr;
  }

  antlrcpp::Any visitImplicationConstraint(
      antlrcpp::UVLcppParser::ImplicationConstraintContext *ctx) {
    visit(ctx->constraint(0));
    auto left = std::move(constraint_stack.top());
    constraint_stack.pop();
    visit(ctx->constraint(1));
    auto right = std::move(constraint_stack.top());
    constraint_stack.pop();

    auto impl_constraint = std::make_unique<vara::feature::ImpliesConstraint>(
        std::move(left), std::move(right));
    constraint_stack.push(std::move(impl_constraint));
    return nullptr;
  }
  antlrcpp::Any
  visitFeatureModel(antlrcpp::UVLcppParser::FeatureModelContext *ctx) override {
    std::string featureName = ctx->getText();

    current_parent = "root";
    parent_stack.emplace("root");
    visitChildren(ctx);
    return true;
  }
};

// argv[1] should be the file path to the uvl file
int main(int argc, char *argv[]) {
  std::string line;
  if (argv[1] != NULL) {
    line = argv[1];
  } else {
    line = "path to uvl file";
  }
  std::ifstream antlr_file(line);
  std::cout << "Opening file" << std::endl;
  if (antlr_file.is_open()) {
    ANTLRInputStream input(antlr_file);
    antlrcpp::UVLcppLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();

    antlrcpp::UVLcppParser parser(&tokens);
    tree::ParseTree *tree = parser.featureModel();

    FeatureVisitor visitor;
    visitor.visit(tree);
    auto test = visitor.FMB.getParentName("FeatureB");
    auto FM = visitor.FMB.buildFeatureModel();
  } else {
    std::cout << "Invalid uvl file " << std::endl;
  }
}
#endif


// handled by cmakelists of upper directory
// #include <vara/Feature/FeatureModel.h>
// #include "vara/Feature/Feature.h"

#include <vara/Feature/Feature.h>
#include <vara/Feature/FeatureModel.h>
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

class FeatureVisitor : public antlrcpp::UVLcppBaseVisitor {
public:
  struct Feature {
    std::string name;
    std::vector<std::string> children;
  };

  std::vector<Feature> features;

  antlrcpp::Any visitFile(antlrcpp::UVLcppParser::FeatureModelContext *ctx) {
    std::cout << "Visited feature node!" << std::endl;

    /*for (auto element : ctx->features()->feature()) {
      std::cout<<"EHRE "<<std::endl;
      std::cout<< element->toString() << std::endl;
      //std::any_cast<vara::feature::FeatureModel>(visitAction(element));
      //elements.push_back(el);
    }*/

    // std::any result = Scene(ctx->name()->NAME()->getText(), elements);

    // return result;
    return visitChildren(ctx);
  }
};

int main(int argc, char *argv[]) {
  std::string line;
  std::ifstream antlrFile("/home/taqi457/Aatir/vara-feature/vara-feature/lib/"
                          "Feature/UVL/testuvl.txt");
  std::cout << "Opening file" << std::endl;
  if (antlrFile.is_open()) {
    ANTLRInputStream input(antlrFile);
    antlrcpp::UVLcppLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();
    for (auto token : tokens.getTokens()) {
      std::cout << token->toString() << std::endl;
    }

    antlrcpp::UVLcppParser parser(&tokens);
    antlrcpp::UVLcppParser::FeatureModelContext *tree = parser.featureModel();
    // tree::ParseTree *tree = parser.featureModel();
    FeatureVisitor visitor;

    // std::string feature =
    // std::any_cast<std::string>(visitor.visitFile(tree));
    std::cout << tree->getText() << std::endl;
    /*for (auto element : tree->features()) {
      std::cout<< element->toString() << std::endl;
      //std::any_cast<vara::feature::FeatureModel>(visitAction(element));
      //elements.push_back(el);
    }*/
    // std::cout << tree->toStringTree(&parser) << std::endl;
    //  modelicaFile.close();
    // auto model =
    // std::any_cast<vara::feature::Feature>(visitor.visitFeature(tree));
  }
}
#endif

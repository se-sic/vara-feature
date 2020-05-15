#include "vara/Feature/FeatureModelParser.h"

#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
  if (argc == 1) {
    std::ifstream DocFile(argv[1]);
    if (DocFile.is_open()) {
      std::string DocRaw((std::istreambuf_iterator<char>(DocFile)),
                         std::istreambuf_iterator<char>());
      DocFile.close();
      std::unique_ptr<vara::feature::FeatureModel> FM =
          vara::feature::FeatureModelXmlParser(DocRaw).buildFeatureModel();
      llvm::ViewGraph(FM.get(), llvm::Twine(FM->getName()));
      return 0;
    }
  }
  return 1;
}

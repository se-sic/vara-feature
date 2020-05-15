#include "vara/Feature/FeatureModelParser.h"

#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
  if (argc > 1) {
    std::unique_ptr<vara::feature::FeatureModel> FM;
    std::ifstream DocFile(argv[1]);
    if (!DocFile.is_open()) {
      return 1;
    }
    std::string DocRaw((std::istreambuf_iterator<char>(DocFile)),
                       std::istreambuf_iterator<char>());
    DocFile.close();
    if (argc > 2) {
      std::ifstream DtdFile(argv[2]);
      if (!DtdFile.is_open()) {
        return 1;
      }
      std::string DtdRaw((std::istreambuf_iterator<char>(DtdFile)),
                         std::istreambuf_iterator<char>());
      DtdFile.close();
      FM = vara::feature::FeatureModelXmlParser(DocRaw, DtdRaw)
               .buildFeatureModel();
    } else {
      FM = vara::feature::FeatureModelXmlParser(DocRaw).buildFeatureModel();
    }
    llvm::ViewGraph(FM.get(), llvm::Twine(FM->getName()));
    return 0;
  }
  return 1;
}

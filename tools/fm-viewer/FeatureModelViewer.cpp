#include "vara/Feature/XmlParser.h"

int main(int argc, char **argv) {
  if (argc > 2) {
    auto P = vara::feature::XmlParser(argv[1], argv[2]);
    if (P.parse()) {
      std::unique_ptr<vara::feature::FeatureModel> FM = P.buildFeatureModel();
      //      FM->dump();
      llvm::ViewGraph(FM.get(), llvm::Twine(FM->getName()));
      return 0;
    }
  }
  return 1;
}

#include "vara/Feature/XmlParser.h"

int main(/*int argc, char ** argv*/) {
  auto P = vara::feature::XmlParser(
      "../../../vara/external/vara-feature/test/test.xml",
      "../../../vara/external/vara-feature/vm.dtd");
  if (P.parse()) {
    std::unique_ptr<vara::feature::FeatureModel> FM = P.buildFeatureModel();
    FM->dump();
    llvm::ViewGraph(FM.get(), llvm::Twine(FM->getName()));
    return 0;
  }
  return 1;
}

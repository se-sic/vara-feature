#ifndef VARA_FEATURE_XMLPARSER_H
#define VARA_FEATURE_XMLPARSER_H

#include "vara/Feature/FeatureModel.h"

#include "libxml/parser.h"
#include "libxml/tree.h"

#include <memory>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               XmlParser Class
//===----------------------------------------------------------------------===//

class XmlParser {
  static constexpr xmlChar OPTIONAL[] = "optional";

  using constXmlCharPtr = const xmlChar *;

private:
  std::string DocPath;
  std::string DtdPath;
  std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)> Dtd;
  std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> Doc;
  std::string VM;
  FeatureModel::FeatureMapTy Features;
  FeatureModel::ConstraintsTy Constraints;
  std::vector<std::pair<std::string, std::string>> RawEdges;
  std::vector<std::pair<std::string, std::string>> RawExcludes;
  std::vector<std::vector<std::pair<std::string, bool>>> RawConstraints;
  void parseConfigurationOption(xmlNode *N, bool Num);
  void parseOptions(xmlNode *N, bool Num);
  void parseConstraints(xmlNode *N);
  void parseVm(xmlNode *N);
  bool parseDtd(const std::string &Filename);
  bool parseDoc(const std::string &Filename);

public:
  explicit XmlParser(std::string DocPath, std::string DtdPath)
      : DocPath(std::move(DocPath)), DtdPath(std::move(DtdPath)),
        Dtd(nullptr, nullptr), Doc(nullptr, nullptr) {}

  /// Parse xml representation of feature model (optional validation with dtd).
  ///
  /// \returns true if successful
  bool parse() {
    return (!DtdPath.empty() ? parseDtd(DtdPath) : true) && parseDoc(DocPath);
  }

  /// Build \a FeatureModel after parsing.
  ///
  /// \returns an instance of \a FeatureModel
  std::unique_ptr<FeatureModel> buildFeatureModel();
};
} // namespace vara::feature

#endif // VARA_FEATURE_XMLPARSER_H
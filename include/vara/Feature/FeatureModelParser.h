#ifndef VARA_FEATURE_FEATUREMODELPARSER_H
#define VARA_FEATURE_FEATUREMODELPARSER_H

#include "vara/Feature/FeatureModel.h"

#include "libxml/parser.h"
#include "libxml/tree.h"

#include <memory>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               FeatureModelParser Class
//===----------------------------------------------------------------------===//

/// \brief Base class for parsers with different input formats.
class FeatureModelParser {
protected:
  explicit FeatureModelParser() = default;

public:
  virtual ~FeatureModelParser() = default;

  /// Build \a FeatureModel after parsing. May return null if parsing or
  /// building failed.
  ///
  /// \returns an instance of \a FeatureModel or \a nullptr
  virtual std::unique_ptr<FeatureModel> buildFeatureModel() = 0;

  /// Checks whether input is a valid feature model as acyclic graph with unique
  /// nodes and tree like structure. Tests precondition of \a buildFeatureModel.
  virtual bool verifyFeatureModel() = 0;
};

//===----------------------------------------------------------------------===//
//                               FeatureModelXmlParser Class
//===----------------------------------------------------------------------===//

/// \brief Parsers for feature models in XML.
class FeatureModelXmlParser : public FeatureModelParser {
public:
  explicit FeatureModelXmlParser(std::string Xml) : Xml(std::move(Xml)) {}

  std::unique_ptr<FeatureModel> buildFeatureModel() override;

  bool verifyFeatureModel() override;

private:
  using constXmlCharPtr = const xmlChar *;

  std::string Xml;
  FeatureModelBuilder FMB;

  bool parseConfigurationOption(xmlNode *Node, bool Num);
  bool parseOptions(xmlNode *Node, bool Num);
  bool parseConstraints(xmlNode *Node);
  bool parseVm(xmlNode *Node);

  static FeatureSourceRange::FeatureSourceLocation
  createFeatureSourceLocation(xmlNode *Node);

  std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> parseDoc();
  static std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)> createDtd();
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELPARSER_H

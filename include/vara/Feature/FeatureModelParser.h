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

  inline static const std::string DtdRaw =
      "<!ELEMENT vm (binaryOptions, numericOptions?, booleanConstraints?, "
      "nonBooleanConstraints?, mixedConstraints?)>\n"
      "<!ATTLIST vm name CDATA #REQUIRED root CDATA #IMPLIED>\n"
      "<!ELEMENT binaryOptions (configurationOption*)>\n"
      "<!ELEMENT numericOptions (configurationOption*)>\n"
      "<!ELEMENT booleanConstraints (constraint*)>\n"
      "<!ELEMENT nonBooleanConstraints (constraint*)>\n"
      "<!ELEMENT mixedConstraints (constraint*)>\n"
      "<!ELEMENT configurationOption (name, outputString?, (prefix | preFix)?, "
      "(postfix | postFix)?, parent?, children?, impliedOptions?, "
      "excludedOptions?, defaultValue?, optional?, ((minValue, maxValue) | "
      "values)?, "
      "stepFunction?, location?)>\n"
      "<!ELEMENT constraint (#PCDATA)>\n"
      "<!ATTLIST constraint req CDATA #IMPLIED exprKind CDATA #IMPLIED>\n"
      "<!ELEMENT name (#PCDATA)>\n"
      "<!ELEMENT outputString (#PCDATA)>\n"
      "<!ELEMENT prefix (#PCDATA)>\n"
      "<!ELEMENT preFix (#PCDATA)>\n"
      "<!ELEMENT postfix (#PCDATA)>\n"
      "<!ELEMENT postFix (#PCDATA)>\n"
      "<!ELEMENT parent (#PCDATA)>\n"
      "<!ELEMENT children (options*)>\n"
      "<!ELEMENT impliedOptions (options*)>\n"
      "<!ELEMENT excludedOptions (options*)>\n"
      "<!ELEMENT defaultValue (#PCDATA)>\n"
      "<!ELEMENT options (#PCDATA)>\n"
      "<!ELEMENT optional (#PCDATA)>\n"
      "<!ELEMENT minValue (#PCDATA)>\n"
      "<!ELEMENT maxValue (#PCDATA)>\n"
      "<!ELEMENT values (#PCDATA)>\n"
      "<!ELEMENT stepFunction (#PCDATA)>\n"
      "<!ELEMENT location (path, start?, end?)>\n"
      "<!ELEMENT path (#PCDATA)>\n"
      "<!ELEMENT start (line, column)>\n"
      "<!ELEMENT end (line, column)>\n"
      "<!ELEMENT line (#PCDATA)>\n"
      "<!ELEMENT column (#PCDATA)>";

  static constexpr xmlChar NAME[] = "name";
  static constexpr xmlChar OPTIONAL[] = "optional";
  static constexpr xmlChar PARENT[] = "parent";
  static constexpr xmlChar EXCLUDEDOPTIONS[] = "excludedOptions";
  static constexpr xmlChar OPTIONS[] = "options";
  static constexpr xmlChar LOCATION[] = "location";
  static constexpr xmlChar PATH[] = "path";
  static constexpr xmlChar START[] = "start";
  static constexpr xmlChar END[] = "end";
  static constexpr xmlChar MINVALUE[] = "minValue";
  static constexpr xmlChar MAXVALUE[] = "maxValue";
  static constexpr xmlChar VALUES[] = "values";
  static constexpr xmlChar CONFIGURATIONOPTION[] = "configurationOption";
  static constexpr xmlChar CONSTRAINT[] = "constraint";
  static constexpr xmlChar ROOT[] = "root";
  static constexpr xmlChar BINARYOPTIONS[] = "binaryOptions";
  static constexpr xmlChar NUMERICOPTIONS[] = "numericOptions";
  static constexpr xmlChar BOOLEANCONSTRAINTS[] = "booleanConstraints";
  static constexpr xmlChar LINE[] = "line";
  static constexpr xmlChar COLUMN[] = "column";

  std::string Xml;
  FeatureModelBuilder FMB;

  bool parseConfigurationOption(xmlNode *N, bool Num);
  bool parseOptions(xmlNode *N, bool Num);
  bool parseConstraints(xmlNode *N);
  bool parseVm(xmlNode *N);

  static FeatureSourceRange::FeatureSourceLocation
  createFeatureSourceLocation(xmlNode *N);

  std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> parseDoc();
  static std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)> createDtd();
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELPARSER_H

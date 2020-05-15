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

class FeatureModelParser {
protected:
  explicit FeatureModelParser() = default;

public:
  /// Build \a FeatureModel after parsing.
  ///
  /// \returns an instance of \a FeatureModel
  virtual std::unique_ptr<FeatureModel> buildFeatureModel() = 0;

  /// Verify \a FeatureModel.
  ///
  /// \returns if \a FeatureModel is valid
  virtual bool verifyFeatureModel() = 0;
};

//===----------------------------------------------------------------------===//
//                               FeatureModelXmlParser Class
//===----------------------------------------------------------------------===//

class FeatureModelXmlParser : public FeatureModelParser {
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

  using constXmlCharPtr = const xmlChar *;

private:
  std::string DocRaw;
  std::optional<std::string> DtdRaw;
  std::string VM;
  std::filesystem::path RootPath;
  FeatureModel::FeatureMapTy Features;
  FeatureModel::ConstraintsTy Constraints;
  std::vector<std::pair<std::string, std::string>> RawEdges;
  std::vector<std::pair<std::string, std::string>> RawExcludes;
  std::vector<std::vector<std::pair<std::string, bool>>> RawConstraints;

  void parseConfigurationOption(xmlNode *N, bool Num);
  void parseOptions(xmlNode *N, bool Num);
  void parseConstraints(xmlNode *N);
  void parseVm(xmlNode *N);

  static Location::LineColumnOffset createLineColumnOffset(xmlNode *N);

  std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> parseDoc();
  std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)> parseDtd();

public:
  explicit FeatureModelXmlParser(
      std::string DocRaw, std::optional<std::string> DtdRaw = std::nullopt)
      : DocRaw(std::move(DocRaw)), DtdRaw(std::move(DtdRaw)) {}

  std::unique_ptr<FeatureModel> buildFeatureModel() override;

  bool verifyFeatureModel() override;
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELPARSER_H

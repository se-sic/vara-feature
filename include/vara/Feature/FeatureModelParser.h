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

//===----------------------------------------------------------------------===//
//                         FeatureModelSxfmParser Class
//===----------------------------------------------------------------------===//

/// \brief Parsers for feature models in SXFM.
/// The SXFM (simple XML feature model) format is a feature model format embeded
/// in an XML structure.
class FeatureModelSxfmParser : public FeatureModelParser {
public:
  using UniqueXmlDoc = std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)>;
  using UniqueXmlDtd = std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)>;

  explicit FeatureModelSxfmParser(std::string Sxfm) : Sxfm(std::move(Sxfm)) {}

  /// This method checks if the given feature model is valid
  ///
  /// \returns true iff the feature model is valid
  bool verifyFeatureModel() override { return parseDoc().get(); }

  /// Reads in and returns the feature model in the sxfm format
  ///
  /// \returns the feature model that was read in
  std::unique_ptr<FeatureModel> buildFeatureModel() override;

private:

  std::string Sxfm;
  FeatureModelBuilder FMB;
  std::string Indentation = "\t";

  /// Returns a pointer to the dtd representation of the xml file, which
  /// is needed to verify the structure of the xml file.
  ///
  /// \returns a pointer to the dtd representation
  static std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)> createDtd();

  /// Parses the given xml file by using libxml2 and returns a pointer to
  /// the xml document.
  ///
  /// \returns a pointer to the xml document
  std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> parseDoc();

  /// Processes the xml tags and its contents.
  ///
  /// \param Node the pointer to the root node
  ///
  /// \returns true iff parsing the contents of the xml tags was successful
  bool parseVm(xmlNode *Node);

  /// Processes the feature tree embedded in the xml file.
  ///
  /// \param FeatureTree the pointer to the feature tree string.
  ///
  /// \returns true iff parsing and processing the whole feature tree was
  /// successful
  bool parseFeatureTree(xmlChar *FeatureTree);

  /// Processes the constraints (i.e., cross-tree constraints) embedded in the
  /// xml file.
  ///
  /// \param Constraints the pointer to the constraint string
  ///
  /// \returns true iff parsing and processing the constraints was successful
  static bool parseConstraints(xmlChar *Constraints);

  /// This method extracts the cardinality from the given line.
  /// The cardinality is wrapped in square brackets (e.g., [1,1])
  ///
  /// \param StringToExtractFrom the string to extract the cardinality from
  ///
  /// \returns returns the cardinality of the given string and is empty if the
  /// format of the string is wrong
  static std::optional<std::tuple<int, int>>
  extractCardinality(const string &StringToExtractFrom);

  /// This method parses the given cardinality and returns an optional.
  /// If the optional is empty, the process failed; otherwise the result
  /// contains either UINT_MAX for the wildcard or the cardinality number as
  /// integer.
  ///
  /// \param CardinalityString the cardinality to parse
  ///
  /// \returns an optional that contains no integer in case of failure or
  /// UINT_MAX for wildcard, or the number itself.
  static std::optional<int> parseCardinality(const string &CardinalityString);
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELPARSER_H

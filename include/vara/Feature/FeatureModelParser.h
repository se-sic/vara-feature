#ifndef VARA_FEATURE_FEATUREMODELPARSER_H
#define VARA_FEATURE_FEATUREMODELPARSER_H

#include "vara/Feature/FeatureModelBuilder.h"

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
  using UniqueXmlDoc = std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)>;
  using UniqueXmlDtd = std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)>;
  using UniqueXmlChar = std::unique_ptr<xmlChar, void (*)(void *)>;

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

  /// This method is solely relevant for parsing XML, as alternatives are
  /// represented als mutual excluded but non-optional features (which requires
  /// additional processing).
  static bool detectXMLAlternatives(FeatureModel &FM);

private:
  std::string Xml;
  FeatureModelBuilder FMB;

  bool parseConfigurationOption(xmlNode *Node, bool Num);
  bool parseOptions(xmlNode *Node, bool Num);
  bool parseConstraints(xmlNode *Node);
  bool parseVm(xmlNode *Node);

  static FeatureSourceRange::FeatureSourceLocation
  createFeatureSourceLocation(xmlNode *Node);
  static FeatureSourceRange createFeatureSourceRange(xmlNode *Head);

  UniqueXmlDoc parseDoc();
  static UniqueXmlDtd createDtd();
};

//===----------------------------------------------------------------------===//
//                         FeatureModelSxfmParser Class
//===----------------------------------------------------------------------===//

/// \brief Parsers for feature models in SXFM.
///
/// \section SXFM
///
/// SXFM stands for Simple XML Feature Model and represents a format that is
/// used to store feature models. In general, feature models consist of a
/// feature model in a tree-like format and cross-tree constraints (i.e.,
/// constraints that would violate the tree-like feature). The SXFM format also
/// distinguishes between a feature tree and cross-tree-constraints. The feature
/// tree is embeded as a tabulator indented text in an XML tag called
/// \c `feature_tree` whereas the cross-tree-constraints are embeded in XML xml
/// tag called \c `constraint`. A brief example and description of the SXFM
/// format can be found at SPLOT (go to `Edit an existing model` -> `SXFM
/// format`). However, they have not given a formal description of the syntax in
/// form of a syntax diagram. Note that the SXFM format does only support binary
/// features.
///
/// \subsection FeatureTree
///
/// In the following, we show the syntax diagram for the syntax of feature
/// trees.
///
/// \image html feature_tree.png
///
/// The item \c `tabulator` stands for the tabulator symbol.
///
/// By using the sequence diagram generator, the syntax diagrams can be
/// generated when using the following rules:
///
/// \code{unparsed}
/// macro_rules! sxfm_feature_tree {
///    (:r $name:expr $id:ident) => { ... };
///    (:r $name:expr) => { ... };
///    ($tabulator:item $($tabulator:item)* :o $name:expr $id:ident ) => { ...
///    };
///    ($tabulator:item $($tabulator:item)* :o $name:expr ) => { ... };
///    ($tabulator:item $($tabulator:item)* :m $name:expr $id:ident) => { ... };
///    ($tabulator:item $($tabulator:item)* :m $name:expr ) => { ... };
///    ($tabulator:item $($tabulator:item)* :g [1..*] ) => { ... };
///    ($tabulator:item $($tabulator:item)* :g [1..1] ) => { ... };
///    ($tabulator:item $($tabulator:item)* : $name:expr ) => { ... };
///    ($tabulator:item $($tabulator:item)* : $name:expr $id:ident ) => { ... };
///}
/// \endcode
///
/// Note that the tabulators indicate the level of the feature. Only the root
/// feature is on level 0 (all other features have a higher indentation). There
/// are different kind of features:
///
/// \li \c `:r` the root feature
/// \li \c `:o` the optional feature
/// \li \c `:m` the mandatory feature
/// \li \c `:g` the parent node of an or group
/// \li \c `:` child of an or group
///
/// Note that the or-group features (\c ':g' ) have no name.
///
/// \subsection Constraints
///
/// \image html constraints.png
///
/// or alternatively:
/// \code{unparsed}
/// macro_rules! sxfm_constraints {
///     ($name:ident : $cnf_formula:tt) => {...};
/// }
/// \endcode
///
/// The constraints have to be provided in CNF format.
/// Therebey, \c `~` stands for negation, \c `or` for a disjunction.
/// Each constraint is written in a separate XML tag \c `constraints`.
///
/// @see[SPLOT](http://www.splot-research.org/sxfm.html)
/// @see[SequenceDiagramGenerator](https://lukaslueg.github.io/macro_railroad_wasm_demo/)
class FeatureModelSxfmParser : public FeatureModelParser {
public:
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
  /// Returns a pointer to the dtd representation of the xml file, which
  /// is needed to verify the structure of the xml file.
  ///
  /// \returns a pointer to the dtd representation
  static UniqueXmlDtd createDtd();

  /// Parses the given xml file by using libxml2 and returns a pointer to
  /// the xml document.
  ///
  /// \returns a pointer to the xml document
  UniqueXmlDoc parseDoc();

  /// Processes the xml tags and its contents.
  ///
  /// \param Node the pointer to the root node
  ///
  /// \returns true iff parsing the contents of the xml tags was successful
  bool parseVm(xmlNode *Node);

  /// Processes the feature tree embedded in the xml file.
  ///
  /// \param FeatureTree the pointer to the node containing the feature tree
  /// string.
  ///
  /// \returns true iff parsing and processing the whole feature tree was
  /// successful
  bool parseFeatureTree(xmlNode *FeatureTree);

  /// Processes the constraints (i.e., cross-tree constraints) embedded in the
  /// xml file.
  ///
  /// \param Constraints the node containing the constraint string
  ///
  /// \returns true iff parsing and processing the constraints was successful
  bool parseConstraints(xmlNode *Constraints);

  /// This method extracts the cardinality from the given line.
  /// The cardinality is wrapped in square brackets (e.g., [1,1])
  ///
  /// \param StringToExtractFrom the string to extract the cardinality from
  ///
  /// \returns the cardinality of the given string and is empty if the
  /// format of the string is wrong
  static std::optional<std::tuple<int, int>>
  extractCardinality(llvm::StringRef StringToExtractFrom);

  /// This method parses the given cardinality and returns an optional.
  /// If the optional is empty, the process failed; otherwise the result
  /// contains either UINT_MAX for the wildcard or the cardinality number as
  /// integer.
  ///
  /// \param CardinalityString the cardinality to parse
  ///
  /// \returns an optional that contains no integer in case of failure or
  /// UINT_MAX for wildcard, or the number itself.
  static std::optional<int> parseCardinality(llvm::StringRef CardinalityString);

  std::string Sxfm;
  FeatureModelBuilder FMB;
  std::string Indentation = "\t";
  std::map<std::string, std::string> IdentifierMap =
      std::map<std::string, std::string>();
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELPARSER_H

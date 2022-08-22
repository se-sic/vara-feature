#ifndef VARA_FEATURE_FEATUREMODELWRITER_H
#define VARA_FEATURE_FEATUREMODELWRITER_H

#include "vara/Feature/FeatureModel.h"

#include "libxml/tree.h"
#include "libxml/xmlwriter.h"

#include <memory>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               FeatureModelParser Class
//===----------------------------------------------------------------------===//

/// \brief Base class for parsers with different input formats.
class FeatureModelWriter {
protected:
  explicit FeatureModelWriter() = default;

public:
  virtual ~FeatureModelWriter() = default;

  /// Build \a FeatureModel after parsing. May return null if parsing or
  /// building failed.
  ///
  /// \returns an instance of \a FeatureModel or \a nullptr
  virtual int writeFeatureModel(std::string Path) = 0;
  virtual llvm::Optional<std::string> writeFeatureModel() = 0;
};

//===----------------------------------------------------------------------===//
//                               FeatureModelXmlParser Class
//===----------------------------------------------------------------------===//

/// \brief Parsers for feature models in XML.
class FeatureModelXmlWriter : public FeatureModelWriter {
public:
  explicit FeatureModelXmlWriter(const FeatureModel &Fm) : Fm{Fm} {}

  int writeFeatureModel(std::string Path) override;
  llvm::Optional<std::string> writeFeatureModel() override;

private:
  int writeFeatureModel(xmlTextWriterPtr Writer);
  int writeVm(xmlTextWriterPtr Writer);
  int writeBinaryFeatures(xmlTextWriterPtr Writer);
  int writeNumericFeatures(xmlTextWriterPtr Writer);
  int writeBooleanConstraints(xmlTextWriterPtr Writer);
  static int writeFeature(xmlTextWriterPtr Writer, Feature &Feature1);
  static int writeSourceRange(xmlTextWriterPtr Writer,
                              FeatureSourceRange &Location);

  const FeatureModel &Fm;
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELWRITER_H

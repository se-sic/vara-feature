#ifndef BDD_SAMPLE_PARSE_FM_H
#define BDD_SAMPLE_PARSE_FM_H

#include "vara/Feature/FeatureModel.h"

#include <memory>
#include <string>

namespace bdd::sample {

     /// \brief Parses a feature model from a XML file and returns the built model
     ///
     /// Reads \p FilePath and runs VaRA's XML parser on the content, verifies it and build a FeatureModel object.
     ///
     /// \param FilePath Path to the XML file
     ///
     /// \return Built feature model if successful, else a nullptr.
     std::unique_ptr<vara::feature::FeatureModel> ParseXML(const std::string &FilePath); //NOLINT Invalid case style for function 'ParseXML'

} // namespace bdd::sample

#endif // BDD_SAMPLE_PARSE_FM_H
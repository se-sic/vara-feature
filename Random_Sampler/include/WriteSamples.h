#ifndef BDD_SAMPLE_WRITE_SAMPLES_H
#define BDD_SAMPLE_WRITE_SAMPLES_H

#include "oxidd/bdd.hpp"

#include <string>
#include <vector>

namespace bdd::sample {

    /// \brief Writes a sample of configurations to a CSV file
    ///
    /// The first row is the feature name header, while each subsequent row is one configuration encoded as 0/1 per feature.
    /// The root feature (Feature 0) is skipped for readability.
    ///
    /// \param Manager The BDD manager containing the feature names
    /// \param Sample The list of configurations needed to be written to the CSV file
    /// \param FilePath The target file path
    void WriteSampleCSV(const oxidd::bdd_manager &Manager, const std::vector<std::vector<bool>> &Sample, std::string &FilePath); //NOLINT Invalid case style for function 'WriteSampleCSV'

} //namespace bdd::sample

#endif // BDD_SAMPLE_WRITE_SAMPLES_H
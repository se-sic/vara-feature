#ifndef BDD_SAMPLE_WRITE_FREQUENCIES_H
#define BDD_SAMPLE_WRITE_FREQUENCIES_H

#include "oxidd/bdd.hpp"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace bdd::sample {

    /// \brief Counts unique configurations in \p Samples and writes the frequency of each of these to a CSV for uniform random sampling
    /// verification.
    ///
    /// Output: Two-column CSV with headers being ConfigID and Count. IDs are sequential starting from 0.
    ///
    /// \param FinalBDD Used only to validate that each configuration in \p Samples satisfies the feature model
    /// \param Samples Configurations to count
    ///
    void WriteFrequencyCSV(const oxidd::bdd_function &FinalBDD, const std::vector<std::vector<bool>> &Samples); //NOLINT Invalid case style for function 'WriteFrequencyCSV'

} //namespace bdd::sample

#endif // BDD_SAMPLE_WRITE_FREQUENCIES_H
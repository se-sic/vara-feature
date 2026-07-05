#ifndef BDD_SAMPLE_WRITE_SAMPLES_H
#define BDD_SAMPLE_WRITE_SAMPLES_H

#include "Plotter.h"
#include "oxidd/bdd.hpp"
#include "string"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace bdd::sample {

    void WriteSampleCSV(const oxidd::bdd_manager &Manager, const std::vector<std::vector<bool>> &Sample, std::string &FilePath); //NOLINT

} //namespace bdd::sample

#endif // BDD_SAMPLE_WRITE_SAMPLES_H
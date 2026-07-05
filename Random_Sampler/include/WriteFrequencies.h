#ifndef BDD_SAMPLE_WRITE_FREQUENCIES_H
#define BDD_SAMPLE_WRITE_FREQUENCIES_H

#include "Plotter.h"
#include "oxidd/bdd.hpp"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace bdd::sample {

    void WriteFrequencyCSV(const oxidd::bdd_function &FinalBDD, std::vector<std::vector<bool>> &Samples); //NOLINT

} //namespace bdd::sample

#endif // BDD_SAMPLE_WRITE_FREQUENCIES_H
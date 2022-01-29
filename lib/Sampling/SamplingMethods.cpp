#include "vara/Sampling/SamplingMethods.h"

#include "stats.hpp"

namespace vara::sampling {

std::tuple<double, double, double> getExampleValues() {
  constexpr double Dens1 = stats::dlaplace(1.0, 1.0, 2.0);
  constexpr double Prob1 = stats::plaplace(1.0, 1.0, 2.0);
  constexpr double Quant1 = stats::qlaplace(0.1, 1.0, 2.0);

  return {Dens1, Prob1, Quant1};
}

} // namespace vara::sampling

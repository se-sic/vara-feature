#ifndef BDD_SAMPLE_PARSE_FM_H
#define BDD_SAMPLE_PARSE_FM_H

#include "Plotter.h"
#include "vara/Feature/FeatureModel.h"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <string>

using std::string;

namespace bdd::sample {

     std::unique_ptr<vara::feature::FeatureModel> ParseXML(std::string &FilePath); //NOLINT

} // namespace bdd::sample

#endif // BDD_SAMPLE_PARSE_FM_H
#include "pybind_common.h"
#include "vara/Feature/FeatureModelParser.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <iostream>

namespace vf = vara::feature;
namespace py = pybind11;

void init_xml_parser(py::module &M) {
  py::class_<vf::FeatureModelXmlParser>(M, "FeatureModelXmlParser")
      .def(py::init<std::string>())
      .def("build_feature_model", &vf::FeatureModelXmlParser::buildFeatureModel,
           R"pbdoc(Build a the FeatureModel from the specifed file.

Note: this requires that the xml was parsed before.
)pbdoc");
}

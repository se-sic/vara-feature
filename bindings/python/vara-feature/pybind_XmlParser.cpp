#include "pybind_common.h"
#include "vara/Feature/XmlParser.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <iostream>

namespace vf = vara::feature;
namespace py = pybind11;

void init_xml_parser(py::module &M) {
  py::class_<vf::XmlParser>(M, "XmlParser")
      .def(py::init<std::string, std::string>())
      .def("parse", &vf::XmlParser::parse,
           R"pbdoc(Parse the xml representation of a feature model.  Validity is
only check if a dtd schema was specified for the Model.
)pbdoc")
      .def("build_feature_model", &vf::XmlParser::buildFeatureModel,
           R"pbdoc(Build a the FeatureModel from the specifed file.

Note: this requires that the xml was parsed before.
)pbdoc");
}

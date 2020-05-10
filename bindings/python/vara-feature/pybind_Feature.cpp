#include "pybind_common.h"
#include "vara/Feature/Feature.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <iostream>

namespace vf = vara::feature;
namespace py = pybind11;

void init_feature_module(py::module &M) {
  py::class_<vf::Feature>(M, "Feature")
      .def_property_readonly("name", &vf::Feature::getName,
                             R"pbdoc(The name of the feature.)pbdoc")
      .def("is_optional", &vf::Feature::isOptional,
           R"pbdoc(`True` if the feature is optional.)pbdoc");

  py::class_<vf::BinaryFeature, vf::Feature>(M, "BinaryFeature")
      .def(py::init<std::string, bool>())
      .def("to_string", &vf::BinaryFeature::toString,
           R"pbdoc(Returns the string representation of a BinaryFeature.)pbdoc")
      .def("__str__", &vf::BinaryFeature::toString);

  py::class_<vf::NumericFeature, vf::Feature>(M, "NumericFeature")
      .def(py::init<std::string, bool,
                    std::variant<std::pair<int, int>, std::vector<int>>>())
      .def(
          "to_string", &vf::NumericFeature::toString,
          R"pbdoc(Returns the string representation of a NumericFeature.)pbdoc")
      .def("__str__", &vf::NumericFeature::toString);
}

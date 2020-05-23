#include "pybind11/attr.h"
#include "pybind_common.h"
#include "vara/Feature/Feature.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <iostream>

namespace vf = vara::feature;
namespace py = pybind11;

void init_feature_module_feature(py::module &M) {
  py::class_<vf::Feature>(M, "Feature")
      .def_property_readonly("name", &vf::Feature::getName,
                             R"pbdoc(The name of the feature.)pbdoc")
      .def("is_optional", &vf::Feature::isOptional,
           R"pbdoc(`True` if the feature is optional.)pbdoc")
      .def(
          "is_root", &vf::Feature::isRoot,
          R"pbdoc(`True` if this is the root feature of a `FeatureModel`.)pbdoc")
      //===----------------------------------------------------------------===//
      // Children
      .def(
          "children",
          [](vf::Feature &F) { return py::make_iterator(F.begin(), F.end()); },
          py::keep_alive<0, 1>(), R"pbdoc(Child features)pbdoc")
      .def(
          "__iter__",
          [](vf::Feature &F) { return py::make_iterator(F.begin(), F.end()); },
          py::keep_alive<0, 1>())
      .def("add_child", &vf::Feature::addChild,
           R"pbdoc(Add a sub feature below this feature.)pbdoc")
      .def("is_child", &vf::Feature::isChild,
           R"pbdoc(Checks if a Feature is a child of this one.)pbdoc")
      //===----------------------------------------------------------------===//
      // Parents
      .def(
          "parents",
          [](vf::Feature &F) {
            return py::make_iterator(F.parents_begin(), F.parents_end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Parent features)pbdoc")
      .def("add_parent", &vf::Feature::addParent,
           R"pbdoc(Add a parent feature to this feature.)pbdoc")
      .def("is_parent", &vf::Feature::isParent,
           R"pbdoc(Checks if a Feature is a parent of this one.)pbdoc");
}

void init_feature_module_binary_feature(py::module &M) {
  py::class_<vf::BinaryFeature, vf::Feature>(M, "BinaryFeature")
      .def(py::init<std::string, bool>())
      .def("to_string", &vf::BinaryFeature::toString,
           R"pbdoc(Returns the string representation of a BinaryFeature.)pbdoc")
      .def("__str__", &vf::BinaryFeature::toString);
}

void init_feature_module_numeric_feature(py::module &M) {
  py::class_<vf::NumericFeature, vf::Feature>(M, "NumericFeature")
      .def(py::init<std::string, bool,
                    std::variant<std::pair<int, int>, std::vector<int>>>())
      .def(
          "to_string", &vf::NumericFeature::toString,
          R"pbdoc(Returns the string representation of a NumericFeature.)pbdoc")
      .def("__str__", &vf::NumericFeature::toString);
}

void init_feature_module(py::module &M) {
  init_feature_module_feature(M);
  init_feature_module_binary_feature(M);
  init_feature_module_numeric_feature(M);
}

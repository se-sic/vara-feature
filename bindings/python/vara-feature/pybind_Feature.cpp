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
      .def("__hash__", &vf::Feature::hash)
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
          [](vf::FeatureTreeNode &F) {
            return py::make_iterator(F.begin(), F.end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Child features)pbdoc")
      .def(
          "__iter__",
          [](vf::Feature &F) { return py::make_iterator(F.begin(), F.end()); },
          py::keep_alive<0, 1>())
      .def("is_child", &vf::Feature::hasEdgeTo,
           R"pbdoc(Checks if a Feature is a child of this one.)pbdoc")
      //===----------------------------------------------------------------===//
      // Parent
      .def("parent", &vf::Feature::getParent,
           py::return_value_policy::reference, R"pbdoc(Parent feature)pbdoc")
      .def("is_parent", &vf::Feature::hasEdgeFrom,
           R"pbdoc(Checks if a Feature is a parent of this one.)pbdoc")
      .def_property("location", &vf::Feature::getFeatureSourceRange,
                    &vf::Feature::setFeatureSourceRange,
                    R"pbdoc(The name of the feature.)pbdoc")

      //===----------------------------------------------------------------===//
      // Utility functions
      .def("__str__", &vf::Feature::toString)
      .def("__eq__", &vf::Feature::operator==)
      .def("__lt__", &vf::Feature::operator<);
}

void init_feature_module_binary_feature(py::module &M) {
  py::class_<vf::BinaryFeature, vf::Feature>(M, "BinaryFeature")
      .def(py::init<std::string, bool>())
      .def(py::init<std::string, bool, vara::feature::FeatureSourceRange>())
      .def(py::init<std::string, bool, vara::feature::FeatureSourceRange,
                    vara::feature::Feature *>())
      .def(py::init<std::string, bool, vara::feature::FeatureSourceRange,
                    vara::feature::Feature *,
                    std::vector<vara::feature::FeatureTreeNode *>>())
      .def(
          "to_string", &vf::BinaryFeature::toString,
          R"pbdoc(Returns the string representation of a BinaryFeature.)pbdoc");
}

void init_feature_module_numeric_feature(py::module &M) {
  py::class_<vf::NumericFeature, vf::Feature>(M, "NumericFeature")
      .def(
          py::init<std::string,
                   std::variant<std::pair<int, int>, std::vector<int>>, bool>())
      .def(py::init<std::string,
                    std::variant<std::pair<int, int>, std::vector<int>>, bool,
                    vara::feature::FeatureSourceRange>())
      .def(
          "to_string", &vf::NumericFeature::toString,
          R"pbdoc(Returns the string representation of a NumericFeature.)pbdoc");
}

void init_feature_module(py::module &M) {
  init_feature_module_feature(M);
  init_feature_module_binary_feature(M);
  init_feature_module_numeric_feature(M);
}

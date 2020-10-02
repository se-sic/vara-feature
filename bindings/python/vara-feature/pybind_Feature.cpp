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
          [](vf::Feature &F) {
            return py::make_iterator(F.children_begin(), F.children_end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Child features)pbdoc")
      .def(
          "__iter__",
          [](vf::Feature &F) { return py::make_iterator(F.begin(), F.end()); },
          py::keep_alive<0, 1>())
      .def("is_child", &vf::Feature::isChild,
           R"pbdoc(Checks if a Feature is a child of this one.)pbdoc")
      //===----------------------------------------------------------------===//
      // Parent
      .def("parent", &vf::Feature::getParent,
           py::return_value_policy::reference, R"pbdoc(Parent feature)pbdoc")
      .def("is_parent", &vf::Feature::isParent,
           R"pbdoc(Checks if a Feature is a parent of this one.)pbdoc")
      //===----------------------------------------------------------------===//
      // Excludes
      .def(
          "excludes",
          [](vf::Feature &F) {
            return py::make_iterator(F.excludes_begin(), F.excludes_end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Excluded features)pbdoc")
      .def("is_excluded", &vf::Feature::isExcluded,
           R"pbdoc(Checks if a Feature is excluded by this Feature.)pbdoc")
      //===----------------------------------------------------------------===//
      // Implications
      .def(
          "implications",
          [](vf::Feature &F) {
            return py::make_iterator(F.implications_begin(),
                                     F.implications_end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Implicated features)pbdoc")
      .def("is_implied_by", &vf::Feature::isImplied,
           R"pbdoc(Checks if a Feature is implicated by this Feature.)pbdoc")

      //===----------------------------------------------------------------===//
      // Alternatives
      .def(
          "alternatives",
          [](vf::Feature &F) {
            return py::make_iterator(F.alternatives_begin(),
                                     F.alternatives_end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Alternative features)pbdoc")
      .def(
          "is_alternative", &vf::Feature::isAlternative,
          R"pbdoc(Checks if a Feature is an alternative of this Feature.)pbdoc")

      .def_property("location", &vf::Feature::getFeatureSourceRange,
                    &vf::Feature::setFeatureSourceRange,
                    py::return_value_policy::reference,
                    R"pbdoc(Returns the code location of the feature variable if
                possible, otherwise, `None`.)pbdoc")

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
                    vara::feature::Feature::FeatureSetType>())
      .def(py::init<std::string, bool, vara::feature::FeatureSourceRange,
                    vara::feature::Feature *,
                    vara::feature::Feature::FeatureSetType,
                    vara::feature::Feature::FeatureSetType>())
      .def(py::init<std::string, bool, vara::feature::FeatureSourceRange,
                    vara::feature::Feature *,
                    vara::feature::Feature::FeatureSetType,
                    vara::feature::Feature::FeatureSetType,
                    vara::feature::Feature::FeatureSetType>())
      .def(py::init<std::string, bool, vara::feature::FeatureSourceRange,
                    vara::feature::Feature *,
                    vara::feature::Feature::FeatureSetType,
                    vara::feature::Feature::FeatureSetType,
                    vara::feature::Feature::FeatureSetType,
                    vara::feature::Feature::FeatureSetType>())
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

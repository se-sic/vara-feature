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
          [](vf::Feature &F) {
            return py::make_iterator(F.children_begin(), F.children_end());
          },
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
           R"pbdoc(Checks if a Feature is a parent of this one.)pbdoc")
      //===----------------------------------------------------------------===//
      // Excludes
      .def(
          "excludes",
          [](vf::Feature &F) {
            return py::make_iterator(F.excludes_begin(), F.excludes_end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Excluded features)pbdoc")
      .def(
          "add_exclude", &vf::Feature::addExclude,
          R"pbdoc(Exclude a Feature, i.e., it can not be pressent when this Feature is present.)pbdoc")
      .def("is_excluded", &vf::Feature::isExcluded,
           R"pbdoc(Checks if a Feature is excluded by this Feature.)pbdoc")
      //===----------------------------------------------------------------===//
      // Implications
      .def(
          "implications",
          [](vf::Feature &F) {
            return py::make_iterator(F.excludes_begin(), F.excludes_end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Implicated features)pbdoc")
      .def(
          "add_implication", &vf::Feature::addImplication,
          R"pbdoc(Implicate another Feature, i.e., selecting this feature implies the otherone.)pbdoc")
      .def("implicates", &vf::Feature::implies,
           R"pbdoc(Checks if a Feature is implicated by this Feature.)pbdoc")

      //===----------------------------------------------------------------===//
      // Alternatives
      .def(
          "alternatives",
          [](vf::Feature &F) {
            return py::make_iterator(F.excludes_begin(), F.excludes_end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Alternative features)pbdoc")
      .def("add_alternative", &vf::Feature::addAlternative,
           R"pbdoc(Add a Feature as an alternative to this Feature.)pbdoc")
      .def(
          "is_alternative", &vf::Feature::isAlternative,
          R"pbdoc(Checks if a Feature is an alternative of this Feature.)pbdoc")

      .def(
          "get_location", &vf::Feature::getLocation,
          R"pbdoc(Returns the code location of the feature variable if possible, otherwise, `None`.)pbdoc")

      //===----------------------------------------------------------------===//
      // Utility functions
      .def("__str__", &vf::Feature::toString)
      .def("__eq__", &vf::Feature::operator==)
      .def("__lt__", &vf::Feature::operator<);
}

void init_feature_module_binary_feature(py::module &M) {
  py::class_<vf::BinaryFeature, vf::Feature>(M, "BinaryFeature")
      .def(py::init<std::string, bool>())
      .def(py::init<std::string, bool, vara::feature::Location>())
      .def(
          "to_string", &vf::BinaryFeature::toString,
          R"pbdoc(Returns the string representation of a BinaryFeature.)pbdoc");
}

void init_feature_module_numeric_feature(py::module &M) {
  py::class_<vf::NumericFeature, vf::Feature>(M, "NumericFeature")
      .def(py::init<std::string, bool,
                    std::variant<std::pair<int, int>, std::vector<int>>>())
      .def(
          "to_string", &vf::NumericFeature::toString,
          R"pbdoc(Returns the string representation of a NumericFeature.)pbdoc");
}

void init_feature_module(py::module &M) {
  init_feature_module_feature(M);
  init_feature_module_binary_feature(M);
  init_feature_module_numeric_feature(M);
}

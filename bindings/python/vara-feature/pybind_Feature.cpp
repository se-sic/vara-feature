#include "pybind11/attr.h"
#include "pybind_common.h"
#include "vara/Feature/Feature.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <iostream>

namespace vf = vara::feature;
namespace py = pybind11;

void init_feature_module_feature_tree_node(py::module &M) {
  py::class_<vf::FeatureTreeNode>(M, "FeatureTreeNode")
      //===----------------------------------------------------------------===//
      // Children
      .def(
          "children",
          [](vf::FeatureTreeNode &F) {
            return py::make_iterator(F.begin(), F.end());
          },
          py::keep_alive<0, 1>(), R"pbdoc(Child nodes.)pbdoc")
      .def(
          "__iter__",
          [](vf::FeatureTreeNode &F) {
            return py::make_iterator(F.begin(), F.end());
          },
          py::keep_alive<0, 1>())
      .def("is_child", &vf::FeatureTreeNode::hasEdgeTo,
           R"pbdoc(Checks if a node is a child of this one.)pbdoc")

      //===----------------------------------------------------------------===//
      // Parent
      .def("parent", &vf::FeatureTreeNode::getParent,
           py::return_value_policy::reference, R"pbdoc(Parent feature)pbdoc")
      .def("is_parent", &vf::FeatureTreeNode::hasEdgeFrom,
           R"pbdoc(Checks if a node is a parent of this one.)pbdoc");
}

void init_feature_module_feature(py::module &M) {
  py::class_<vf::Feature, vf::FeatureTreeNode>(M, "Feature")
      .def("__hash__", &vf::Feature::hash)
      .def_property_readonly("name", &vf::Feature::getName,
                             R"pbdoc(The name of the feature.)pbdoc")
      .def(
          "is_root",
          [](const vf::Feature &F) { return llvm::isa<vf::RootFeature>(F); },
          R"pbdoc(`True` if this is the root of a `FeatureModel`.)pbdoc")
      .def("is_optional", &vf::Feature::isOptional,
           R"pbdoc(`True` if the feature is optional.)pbdoc")
      .def_property_readonly(
          "locations",
          [](vf::Feature &F) {
            return py::make_iterator(F.getLocationsBegin(),
                                     F.getLocationsEnd());
          },
          R"pbdoc(The mapped code locations of the feature.)pbdoc")
      .def(
          "hasLocations", &vf::Feature::hasLocations,
          R"pbdoc(Returns True, if the feature has at least one location.)pbdoc")
      .def(
          "addLocation",
          [](vf::Feature &F, vf::FeatureSourceRange &Fsr) {
            F.addLocation(Fsr);
          },
          R"pbdoc(Adds given location to feature.)pbdoc")
      .def(
          "updateLocation",
          [](vf::Feature &F, const vf::FeatureSourceRange &OldFsr,
             vf::FeatureSourceRange &NewFsr) {
            return F.updateLocation(OldFsr, NewFsr);
          },
          R"pbdoc(Replacees first location with second.)pbdoc")
      .def(
          "removeLocation",
          [](vf::Feature &F, const vf::FeatureSourceRange &Fsr) {
            F.removeLocation(Fsr);
          },
          R"pbdoc(Removes given location from feature.)pbdoc")

      //===----------------------------------------------------------------===//
      // Utility functions
      .def("__str__", &vf::Feature::toString)
      .def("__eq__", &vf::Feature::operator==)
      .def("__lt__", &vf::Feature::operator<);
}

void init_feature_module_binary_feature(py::module &M) {
  py::class_<vf::BinaryFeature, vf::Feature>(M, "BinaryFeature")
      .def(py::init<std::string, bool>())
      .def(py::init<std::string, bool,
                    std::vector<vara::feature::FeatureSourceRange>>())
      .def(py::init<std::string, bool,
                    std::vector<vara::feature::FeatureSourceRange>,
                    std::string>());
}

void init_feature_module_numeric_feature(py::module &M) {
  py::class_<vf::NumericFeature, vf::Feature>(M, "NumericFeature")
      .def(py::init<std::string, vf::NumericFeature::ValuesVariantType, bool>())
      .def(py::init<std::string, vf::NumericFeature::ValuesVariantType, bool,
                    std::vector<vara::feature::FeatureSourceRange>>())
      .def(py::init<std::string, vf::NumericFeature::ValuesVariantType, bool,
                    std::vector<vara::feature::FeatureSourceRange>,
                    std::string>());
}

void init_feature_module_root_feature(py::module &M) {
  py::class_<vf::RootFeature, vf::Feature>(M, "RootFeature")
      .def(py::init<std::string>());
}

void init_feature_module(py::module &M) {
  init_feature_module_feature_tree_node(M);
  init_feature_module_feature(M);
  init_feature_module_binary_feature(M);
  init_feature_module_numeric_feature(M);
  init_feature_module_root_feature(M);
}

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
      .def("is_root", &vf::FeatureTreeNode::isRoot,
           R"pbdoc(`True` if this is the root of a `FeatureModel`.)pbdoc")

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
      .def("parent", &vf::FeatureTreeNode::getParent<vf::FeatureTreeNode>,
           py::return_value_policy::reference, R"pbdoc(Parent feature)pbdoc")
      .def("is_parent", &vf::FeatureTreeNode::hasEdgeFrom,
           R"pbdoc(Checks if a node is a parent of this one.)pbdoc");
}

void init_feature_module_feature(py::module &M) {
  py::class_<vf::Feature, vf::FeatureTreeNode>(M, "Feature")
      .def("__hash__", &vf::Feature::hash)
      .def_property_readonly("name", &vf::Feature::getName,
                             R"pbdoc(The name of the feature.)pbdoc")
      .def("is_optional", &vf::Feature::isOptional,
           R"pbdoc(`True` if the feature is optional.)pbdoc")
      .def_property_readonly(
          "locations",
          [](vf::Feature &F) {
            return py::make_iterator(F.getLocationsBegin(),
                                     F.getLocationsEnd());
          },
          R"pbdoc(The mapped code locations of the feature.)pbdoc")
      .def("addLocation",
           [](vf::Feature &F, vf::FeatureSourceRange &Fsr) {
             F.addLocation(Fsr);
           })
      .def("updateLocation",
           [](vf::Feature &F, const vf::FeatureSourceRange &OldFsr,
              vf::FeatureSourceRange &NewFsr) {
             return F.updateLocation(OldFsr, NewFsr);
           })
      .def("removeLocation",
           [](vf::Feature &F, const vf::FeatureSourceRange &Fsr) {
             F.removeLocation(Fsr);
           })

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
                    vara::feature::Feature *>())
      .def(py::init<std::string, bool,
                    std::vector<vara::feature::FeatureSourceRange>,
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
                    std::vector<vara::feature::FeatureSourceRange>>())
      .def(
          "to_string", &vf::NumericFeature::toString,
          R"pbdoc(Returns the string representation of a NumericFeature.)pbdoc");
}

void init_feature_module(py::module &M) {
  init_feature_module_feature_tree_node(M);
  init_feature_module_feature(M);
  init_feature_module_binary_feature(M);
  init_feature_module_numeric_feature(M);
}

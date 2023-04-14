#include "pybind11/attr.h"
#include "pybind_common.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelTransaction.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <filesystem>
#include <iostream>

namespace vf = vara::feature;
namespace py = pybind11;

void init_feature_model_module(py::module &M) {
  py::class_<vf::FeatureModel::BooleanConstraint>(M, "BooleanConstraint")
      .def(py::init([](vf::Constraint &C) {
        return std::make_unique<vf::FeatureModel::BooleanConstraint>(C.clone());
      }))
      .def("__str__", &vf::FeatureModel::BooleanConstraint::toString)
      .def_property_readonly("constraint",
                             &vf::FeatureModel::BooleanConstraint::constraint);

  py::class_<vf::FeatureModel::NonBooleanConstraint>(M, "NonBooleanConstraint")
      .def(py::init([](vf::Constraint &C) {
        return std::make_unique<vf::FeatureModel::NonBooleanConstraint>(
            C.clone());
      }))
      .def("__str__", &vf::FeatureModel::NonBooleanConstraint::toString)
      .def_property_readonly(
          "constraint", &vf::FeatureModel::NonBooleanConstraint::constraint);

  py::enum_<vf::FeatureModel::MixedConstraint::Req>(M, "Req")
      .value("ALL", vf::FeatureModel::MixedConstraint::Req::ALL)
      .value("NONE", vf::FeatureModel::MixedConstraint::Req::NONE);
  py::enum_<vf::FeatureModel::MixedConstraint::ExprKind>(M, "ExprKind")
      .value("POS", vf::FeatureModel::MixedConstraint::ExprKind::POS)
      .value("NEG", vf::FeatureModel::MixedConstraint::ExprKind::NEG);
  py::class_<vf::FeatureModel::MixedConstraint>(M, "MixedConstraint")
      .def(py::init([](vf::Constraint &C,
                       vf::FeatureModel::MixedConstraint::Req R,
                       vf::FeatureModel::MixedConstraint::ExprKind E) {
        return std::make_unique<vf::FeatureModel::MixedConstraint>(C.clone(), R,
                                                                   E);
      }))
      .def("__str__", &vf::FeatureModel::MixedConstraint::toString)
      .def_property_readonly("req", &vf::FeatureModel::MixedConstraint::req)
      .def_property_readonly("exprKind",
                             &vf::FeatureModel::MixedConstraint::exprKind)
      .def_property_readonly("constraint",
                             &vf::FeatureModel::MixedConstraint::constraint);

  py::class_<vf::FeatureModel>(M, "FeatureModel")
      .def_property_readonly(
          "name", &vf::FeatureModel::getName,
          R"pbdoc(Returns the name of the FeatureModel.)pbdoc")
      .def_property_readonly(
          "path",
          [](const vf::FeatureModel &FM) { return FM.getPath().string(); })
      .def_property(
          "commit",
          [](const vf::FeatureModel &FM) { return FM.getCommit().str(); },
          [](vf::FeatureModel &FM, std::string Commit) {
            vf::setCommit(FM, std::move(Commit));
          },
          R"pbdoc(Returns the commit associated to the FeatureModel.)pbdoc")
      .def("get_root", &vf::FeatureModel::getRoot,
           py::return_value_policy::reference,
           R"pbdoc(Returns the root Feature.)pbdoc")
      .def(
          "get_feature",
          [](vf::FeatureModel &FM, const std::string &Name) {
            return FM.getFeature(Name);
          },
          py::return_value_policy::reference, R"pbdoc(Returns Feature.)pbdoc")
      .def(
          "add_binary_feature",
          [](vf::FeatureModel &FM, vf::Feature &Parent, std::string Name,
             bool Opt) {
            vf::addFeature(
                FM, std::make_unique<vf::BinaryFeature>(std::move(Name), Opt),
                &Parent);
          },
          R"pbdoc(Add binary feature with given properties to Feature model.)pbdoc")
      .def(
          "add_binary_feature",
          [](vf::FeatureModel &FM, vf::Feature &Parent, std::string Name,
             bool Opt,
             std::vector<vara::feature::FeatureSourceRange> Locations) {
            vf::addFeature(FM,
                           std::make_unique<vf::BinaryFeature>(
                               std::move(Name), Opt, std::move(Locations)),
                           &Parent);
          },
          R"pbdoc(Add binary feature with given properties to Feature model.)pbdoc")
      .def(
          "add_numeric_feature",
          [](vf::FeatureModel &FM, vf::Feature &Parent, std::string Name,
             vf::NumericFeature::ValuesVariantType Values, bool Opt) {
            vf::addFeature(FM,
                           std::make_unique<vf::NumericFeature>(
                               std::move(Name), std::move(Values), Opt),
                           &Parent);
          },
          R"pbdoc(Add numeric feature with given properties to Feature model.)pbdoc")
      .def(
          "add_numeric_feature",
          [](vf::FeatureModel &FM, vf::Feature &Parent, std::string Name,
             vf::NumericFeature::ValuesVariantType Values, bool Opt,
             std::vector<vara::feature::FeatureSourceRange> Locations) {
            vf::addFeature(FM,
                           std::make_unique<vf::NumericFeature>(
                               std::move(Name), std::move(Values), Opt,
                               std::move(Locations)),
                           &Parent);
          },
          R"pbdoc(Add numeric feature with given properties to Feature model.)pbdoc")
      .def(
          "remove_feature",
          [](vf::FeatureModel &FM, vf::Feature &OldFeature, bool Recursive) {
            vf::removeFeature(FM, &OldFeature, Recursive);
          },
          py::arg(), py::arg("recursive") = false,
          R"pbdoc(Remove Feature from model.)pbdoc")
      .def("merge_with", &vf::mergeFeatureModels, py::arg(),
           py::arg("strict") = true,
           R"pbdoc(Merge with other Feature model.)pbdoc")
      .def("size", &vf::FeatureModel::size,
           R"pbdoc(Returns the amount of Features in the Model.)pbdoc")
      .def(
          "view", &vf::FeatureModel::view,
          R"pbdoc(View the FeatureModel visualized as a graph. This launches a view via xdg-open for dot files.)pbdoc")
      .def("__len__", &vf::FeatureModel::size)
      .def(
          "__iter__",
          [](const vf::FeatureModel &FM) {
            return py::make_iterator(FM.begin(), FM.end());
          },
          py::keep_alive<0, 1>())
      .def_property_readonly("booleanConstraints",
                             [](const vf::FeatureModel &FM) {
                               return py::make_iterator(
                                   FM.booleanConstraints().begin(),
                                   FM.booleanConstraints().end());
                             })
      .def_property_readonly("nonBooleanConstraints",
                             [](const vf::FeatureModel &FM) {
                               return py::make_iterator(
                                   FM.nonBooleanConstraints().begin(),
                                   FM.nonBooleanConstraints().end());
                             })
      .def_property_readonly(
          "mixedConstraints", [](const vf::FeatureModel &FM) {
            return py::make_iterator(FM.mixedConstraints().begin(),
                                     FM.mixedConstraints().end());
          });
  M.def(
      "loadFeatureModel",
      [](const std::filesystem::path &Path) {
        return vf::loadFeatureModel(Path.string());
      },
      R"pbdoc(Load FeatureModel from the given file.

Args:
  path (str): to the FeatureModel file

Returns: the loaded FeatureModel
)pbdoc");
  M.def(
      "verifyFeatureModel",
      [](const std::filesystem::path &Path) -> bool {
        return vf::verifyFeatureModel(Path.string());
      },
      R"pbdoc(Verifies the FeatureModel from the given file.

Args:
  path (str): to the FeatureModel file

Returns: true, if the FeatureModel is valid
)pbdoc");
}

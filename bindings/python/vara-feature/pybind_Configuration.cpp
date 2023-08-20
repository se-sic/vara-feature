#include "pybind_common.h"
#include "vara/Configuration/Configuration.h"
#include "vara/Solver/ConfigurationFactory.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <filesystem>
#include <iostream>

namespace vf = vara::feature;
namespace vs = vara::solver;
namespace py = pybind11;

void init_configuration_module(py::module &M) {
  py::class_<vf::ConfigurationOption>(M, "ConfigurationOption")
      .def_property_readonly(
          "name", [](vf::ConfigurationOption &CO) { return CO.name().str(); },
          R"pbdoc(Returns the name of the ConfigurationOption.)pbdoc")
      .def_property_readonly(
          "value", &vf::ConfigurationOption::asString,
          R"pbdoc(Returns the value of the ConfigurationOption as str.)pbdoc");
  py::class_<vf::Configuration>(M, "Configuration")
      .def("__str__", [](vf::Configuration &C) { return C.dumpToString(); })
      /*.def(
          "__iter__",
          [](vf::Configuration &C) {
            return py::make_iterator(C.begin(), C.end());
          },
          py::keep_alive<0, 1>())*/
      .def("getOptions", [](vf::Configuration &C) {
        std::vector<vf::ConfigurationOption> V;
        for (auto &O : C) {
          V.push_back(*O.getValue());
        }

        return V;
      });

  M.def(
      "getAllConfigs",
      [](vf::FeatureModel &FM) {
        return vs::ConfigurationFactory::getAllConfigs(FM).extractValue();
      },
      R"pbdoc(Get all configurations of FeatureModel.

Args:
  fm (FeatureModel): the FeatureModel

Returns: list of all configurations.
)pbdoc");
}

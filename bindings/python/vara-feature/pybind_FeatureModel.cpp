#include "pybind11/attr.h"
#include "pybind_common.h"
#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <iostream>

namespace vf = vara::feature;
namespace py = pybind11;

void init_feature_model_module(py::module &M) {
  py::class_<vf::FeatureModel>(M, "FeatureModel")
      .def("getName", &vf::FeatureModel::getName,
           R"pbdoc(Returns the name of the FeatureModel.)pbdoc")
      .def("getRoot", &vf::FeatureModel::getRoot,
           py::return_value_policy::reference,
           R"pbdoc(Returns the root Feature.)pbdoc")
      .def("size", &vf::FeatureModel::size,
           R"pbdoc(Returns the amount of Features in the Model.)pbdoc")
      .def(
          "__iter__",
          [](vf::FeatureModel &FM) {
            return py::make_iterator(FM.begin(), FM.end());
          },
          py::keep_alive<0, 1>());
}

#include "pybind11/attr.h"
#include "pybind_common.h"
#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <iostream>

void test2() { std::cout << "Testinger222...\n"; }

namespace vf = vara::feature;
namespace py = pybind11;

void init_feature_model_module(py::module &M) {
  M.def("test", &test2, R"pbdoc()pbdoc");
  py::class_<vf::FeatureModel>(M, "FeatureModel")
      .def("getName", &vf::FeatureModel::getName)
      .def("getRoot", &vf::FeatureModel::getRoot,
           py::return_value_policy::reference)
      .def("size", &vf::FeatureModel::size)
      .def(
          "__iter__",
          [](vf::FeatureModel &FM) {
            return py::make_iterator(FM.begin(), FM.end());
          },
          py::keep_alive<0, 1>());
}

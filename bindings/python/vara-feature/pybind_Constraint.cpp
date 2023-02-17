#include "pybind_common.h"
#include "vara/Feature/ConstraintBuilder.h"

#include "pybind11/pybind11.h"

namespace vf = vara::feature;
namespace py = pybind11;

void init_constraint_module_constraint(py::module &M) {
  py::class_<vf::Constraint>(M, "Constraint")
      .def("__str__", &vf::Constraint::toString);
}

void init_constraint_module_constraint_builder(py::module &M) {
  py::class_<vf::ConstraintBuilder>(M, "ConstraintBuilder")
      .def(py::init<>())
      .def("build", &vf::ConstraintBuilder::build)
      .def("constant", &vf::ConstraintBuilder::constant)
      .def("feature", &vf::ConstraintBuilder::feature)
      .def("left", &vf::ConstraintBuilder::feature);
}

void init_constraint_module(py::module &M) {
  init_constraint_module_constraint(M);
  init_constraint_module_constraint_builder(M);
}

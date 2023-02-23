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
      .def("build", &vf::ConstraintBuilder::build,
           R"pbdoc('Builds constraint.')pbdoc")
      .def("openPar", &vf::ConstraintBuilder::openPar, R"pbdoc('(')pbdoc")
      .def("closePar", &vf::ConstraintBuilder::closePar, R"pbdoc(')')pbdoc")
      .def("constant", &vf::ConstraintBuilder::constant, R"pbdoc('42')pbdoc")
      .def("feature", &vf::ConstraintBuilder::feature, R"pbdoc('Foo')pbdoc")
      .def("lNot", &vf::ConstraintBuilder::lNot, R"pbdoc('!Foo')pbdoc")
      .def("lOr", &vf::ConstraintBuilder::lOr, R"pbdoc('Foo | Bar')pbdoc")
      .def("lXor", &vf::ConstraintBuilder::lXor, R"pbdoc('Foo ^ Bar')pbdoc")
      .def("lAnd", &vf::ConstraintBuilder::lAnd, R"pbdoc('Foo & Bar')pbdoc")
      .def("implies", &vf::ConstraintBuilder::implies,
           R"pbdoc('Foo => Bar')pbdoc")
      .def("excludes", &vf::ConstraintBuilder::excludes,
           R"pbdoc('Foo => !Bar')pbdoc")
      .def("equivalent", &vf::ConstraintBuilder::equivalent,
           R"pbdoc('Foo <=> Bar')pbdoc")
      .def("equal", &vf::ConstraintBuilder::equal, R"pbdoc('Foo = Bar')pbdoc")
      .def("notEqual", &vf::ConstraintBuilder::notEqual,
           R"pbdoc('Foo != Bar')pbdoc")
      .def("less", &vf::ConstraintBuilder::less, R"pbdoc('Foo < Bar')pbdoc")
      .def("greater", &vf::ConstraintBuilder::greater,
           R"pbdoc('Foo > Bar')pbdoc")
      .def("lessEqual", &vf::ConstraintBuilder::lessEqual,
           R"pbdoc('Foo <= Bar')pbdoc")
      .def("greaterEqual", &vf::ConstraintBuilder::greaterEqual,
           R"pbdoc('Foo >= Bar')pbdoc")
      .def("neg", &vf::ConstraintBuilder::neg, R"pbdoc('~Foo')pbdoc")
      .def("add", &vf::ConstraintBuilder::add, R"pbdoc('Foo + Bar')pbdoc")
      .def("subtract", &vf::ConstraintBuilder::subtract,
           R"pbdoc('Foo - Bar')pbdoc")
      .def("multiply", &vf::ConstraintBuilder::multiply,
           R"pbdoc('Foo * Bar')pbdoc")
      .def("divide", &vf::ConstraintBuilder::divide,
           R"pbdoc('Foo / Bar')pbdoc");
}

void init_constraint_module(py::module &M) {
  init_constraint_module_constraint(M);
  init_constraint_module_constraint_builder(M);
}

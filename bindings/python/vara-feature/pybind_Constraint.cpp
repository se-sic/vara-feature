#include "pybind_common.h"
#include "vara/Feature/ConstraintBuilder.h"
#include "vara/Feature/Constraint.h"
#include "pybind11/pybind11.h"

namespace vf = vara::feature;
namespace py = pybind11;

void init_constraint_module_constraint(py::module &M) {
  py::enum_<vf::Constraint::ConstraintKind>(M, "ConstraintKind")
      .value("CK_ADDITION", vf::Constraint::ConstraintKind::CK_ADDITION)
      .value("CK_AND", vf::Constraint::ConstraintKind::CK_AND)
      .value("CK_BINARY", vf::Constraint::ConstraintKind::CK_BINARY)
      .value("CK_DIVISION", vf::Constraint::ConstraintKind::CK_DIVISION)
      .value("CK_EQUAL", vf::Constraint::ConstraintKind::CK_EQUAL)
      .value("CK_EQUIVALENCE", vf::Constraint::ConstraintKind::CK_EQUIVALENCE)
      .value("CK_EXCLUDES", vf::Constraint::ConstraintKind::CK_EXCLUDES)
      .value("CK_FEATURE", vf::Constraint::ConstraintKind::CK_FEATURE)
      .value("CK_GREATER", vf::Constraint::ConstraintKind::CK_GREATER)
      .value("CK_GREATER_EQUAL", vf::Constraint::ConstraintKind::CK_GREATER_EQUAL)
      .value("CK_IMPLIES", vf::Constraint::ConstraintKind::CK_IMPLIES)
      .value("CK_INTEGER", vf::Constraint::ConstraintKind::CK_INTEGER)
      .value("CK_LESS", vf::Constraint::ConstraintKind::CK_LESS)
      .value("CK_LESS_EQUAL", vf::Constraint::ConstraintKind::CK_LESS_EQUAL)
      .value("CK_MULTIPLICATION", vf::Constraint::ConstraintKind::CK_MULTIPLICATION)
      .value("CK_NEG", vf::Constraint::ConstraintKind::CK_NEG)
      .value("CK_NOT", vf::Constraint::ConstraintKind::CK_NOT)
      .value("CK_NOT_EQUAL", vf::Constraint::ConstraintKind::CK_NOT_EQUAL)
      .value("CK_OR", vf::Constraint::ConstraintKind::CK_OR)
      .value("CK_PRIMARY", vf::Constraint::ConstraintKind::CK_PRIMARY)
      .value("CK_SUBTRACTION", vf::Constraint::ConstraintKind::CK_SUBTRACTION)
      .value("CK_UNARY", vf::Constraint::ConstraintKind::CK_UNARY)
      .value("CK_XOR", vf::Constraint::ConstraintKind::CK_XOR)
      .export_values();

  py::class_<vf::Constraint, std::unique_ptr<vf::Constraint, py::nodelete>>(M, "Constraint")
      .def("__str__", &vf::Constraint::toString)
      .def("clone", &vf::Constraint::clone)
      .def("get_kind", &vf::Constraint::getKind)
      .def("set_parent", &vf::Constraint::setParent)
      .def("get_parent", &vf::Constraint::getParent)
      .def("get_root", &vf::Constraint::getRoot)
      .def("to_html", &vf::Constraint::toHTML)
      .def("accept", &vf::Constraint::accept)
      .def("get_left_operand", [](vf::Constraint &self) {
          auto *binaryConstraint = dynamic_cast<vf::BinaryConstraint*>(&self);
          if (binaryConstraint) {
              return binaryConstraint->getLeftOperand();
          }
          throw std::runtime_error("Constraint is not binary");
      })
      .def("get_right_operand", [](vf::Constraint &self) {
          auto *binaryConstraint = dynamic_cast<vf::BinaryConstraint*>(&self);
          if (binaryConstraint) {
              return binaryConstraint->getRightOperand();
          }
          throw std::runtime_error("Constraint is not binary");
      });
}

void init_constraint_module_constraint_builder(py::module &M) {
  py::class_<vf::ConstraintBuilder>(M, "ConstraintBuilder")
      .def(py::init<>())
      .def("build", &vf::ConstraintBuilder::build,
           R"pbdoc('Builds constraint.')pbdoc")
      .def("__call__", &vf::ConstraintBuilder::operator(),
           R"pbdoc('(...)')pbdoc")
      .def("openPar", &vf::ConstraintBuilder::openPar, R"pbdoc('(...')pbdoc")
      .def("closePar", &vf::ConstraintBuilder::closePar, R"pbdoc('...)')pbdoc")
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
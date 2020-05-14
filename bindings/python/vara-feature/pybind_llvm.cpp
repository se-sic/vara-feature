#include "pybind_common.h"
#include "llvm/ADT/StringRef.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

namespace py = pybind11;

void init_llvm(py::module &M) {
  py::class_<llvm::StringRef>(M, "StringRef")
      .def("str", &llvm::StringRef::str,
           R"pbdoc(Convert the StringRef to a string.)pbdoc");
}

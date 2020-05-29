#include "pybind_common.h"
#include "vara/Feature/Location.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include <optional>

#ifdef STD_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace vf = vara::feature;
namespace py = pybind11;

void init_feature_module_location(py::module &M) {
  py::class_<vf::Location>(M, "Location")
      .def(py::init([](std::string Path) {
        return vf::Location(fs::path(std::move(Path)), std::nullopt,
                            std::nullopt);
      }))
      .def(py::init([](std::string Path,
                       std::optional<vf::Location::LineColumnOffset> Start) {
        return vf::Location(fs::path(std::move(Path)), Start, std::nullopt);
      }))
      .def(py::init([](std::string Path,
                       std::optional<vf::Location::LineColumnOffset> Start,
                       std::optional<vf::Location::LineColumnOffset> End) {
        return vf::Location(fs::path(std::move(Path)), Start, End);
      }))
      .def_property_readonly(
          "path", [](vf::Location &Loc) { return Loc.getPath().string(); },
          R"pbdoc(Path to the source file)pbdoc")
      .def("get_start", &vf::Location::getStart,
           R"pbdoc(Get the start `LineColumnOffset` of this `Location`.)pbdoc")
      .def("get_end", &vf::Location::getEnd,
           R"pbdoc(Get the end `LineColumnOffset` of this `Location`.)pbdoc")
      .def("__str__", &vf::Location::toString);

  py::class_<vf::Location::LineColumnOffset>(M, "LineColumnOffset")
      .def(py::init<int, int>())
      .def_property_readonly("line_number",
                             &vf::Location::LineColumnOffset::getLineNumber)
      .def_property_readonly("column_offset",
                             &vf::Location::LineColumnOffset::getColumnOffset)
      .def("__str__", &vf::Location::LineColumnOffset::toString)
      .def("__eq__", [](const vf::Location::LineColumnOffset &Self,
                        const vf::Location::LineColumnOffset &Other) {
        return Self == Other;
      });
}

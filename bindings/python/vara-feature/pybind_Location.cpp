#include "pybind_common.h"
#include "vara/Feature/FeatureSourceRange.h"

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
  py::class_<vf::FeatureSourceRange>(M, "Location")
      .def(py::init([](std::string Path) {
        return vf::FeatureSourceRange(fs::path(std::move(Path)), std::nullopt,
                                      std::nullopt);
      }))
      .def(py::init(
          [](std::string Path,
             std::optional<vf::FeatureSourceRange::LineColumnOffset> Start) {
            return vf::FeatureSourceRange(fs::path(std::move(Path)), Start,
                                          std::nullopt);
          }))
      .def(py::init(
          [](std::string Path,
             std::optional<vf::FeatureSourceRange::LineColumnOffset> Start,
             std::optional<vf::FeatureSourceRange::LineColumnOffset> End) {
            return vf::FeatureSourceRange(fs::path(std::move(Path)), Start,
                                          End);
          }))
      .def_property_readonly(
          "path",
          [](vf::FeatureSourceRange &Loc) { return Loc.getPath().string(); },
          R"pbdoc(Path to the source file)pbdoc")
      .def("get_start", &vf::FeatureSourceRange::getStart,
           R"pbdoc(Get the start `LineColumnOffset` of this `Location`.)pbdoc")
      .def("get_end", &vf::FeatureSourceRange::getEnd,
           R"pbdoc(Get the end `LineColumnOffset` of this `Location`.)pbdoc")
      .def("__str__", &vf::FeatureSourceRange::toString);

  py::class_<vf::FeatureSourceRange::LineColumnOffset>(M, "LineColumnOffset")
      .def(py::init<int, int>())
      .def_property_readonly(
          "line_number",
          &vf::FeatureSourceRange::LineColumnOffset::getLineNumber)
      .def_property_readonly(
          "column_offset",
          &vf::FeatureSourceRange::LineColumnOffset::getColumnOffset)
      .def("__str__", &vf::FeatureSourceRange::LineColumnOffset::toString)
      .def("__eq__", [](const vf::FeatureSourceRange::LineColumnOffset &Self,
                        const vf::FeatureSourceRange::LineColumnOffset &Other) {
        return Self == Other;
      });
}

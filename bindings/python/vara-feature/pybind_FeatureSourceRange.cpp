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
             std::optional<vf::FeatureSourceRange::FeatureSourceLocation>
                 Start) {
            return vf::FeatureSourceRange(fs::path(std::move(Path)), Start,
                                          std::nullopt);
          }))
      .def(py::init(
          [](std::string Path,
             std::optional<vf::FeatureSourceRange::FeatureSourceLocation> Start,
             std::optional<vf::FeatureSourceRange::FeatureSourceLocation> End) {
            return vf::FeatureSourceRange(fs::path(std::move(Path)), Start,
                                          End);
          }))
      .def_property_readonly(
          "path",
          [](vf::FeatureSourceRange &Loc) { return Loc.getPath().string(); },
          R"pbdoc(Path to the source file)pbdoc")
      .def("get_start", &vf::FeatureSourceRange::getStart,
           py::return_value_policy::reference,
           R"pbdoc(Get the start `LineColumnOffset` of this `Location`.)pbdoc")
      .def("get_end", &vf::FeatureSourceRange::getEnd,
           py::return_value_policy::reference,
           R"pbdoc(Get the end `LineColumnOffset` of this `Location`.)pbdoc")
      .def("__str__", &vf::FeatureSourceRange::toString);

  py::class_<vf::FeatureSourceRange::FeatureSourceLocation>(M,
                                                            "LineColumnOffset")
      .def(py::init<int, int>())
      .def_property(
          "line_number",
          &vf::FeatureSourceRange::FeatureSourceLocation::getLineNumber,
          &vf::FeatureSourceRange::FeatureSourceLocation::setLineNumber)
      .def_property(
          "column_offset",
          &vf::FeatureSourceRange::FeatureSourceLocation::getColumnOffset,
          &vf::FeatureSourceRange::FeatureSourceLocation::setColumnOffset)
      .def("__str__", &vf::FeatureSourceRange::FeatureSourceLocation::toString)
      .def("__eq__",
           [](const vf::FeatureSourceRange::FeatureSourceLocation &Self,
              const vf::FeatureSourceRange::FeatureSourceLocation &Other) {
             return Self == Other;
           });
}

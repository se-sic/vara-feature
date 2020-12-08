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

void init_feature_location_module(py::module &M) {
  py::class_<vf::FeatureSourceRange>(M, "Location")
      .def(py::init([](std::string Path) {
        return vf::FeatureSourceRange(std::nullopt, std::nullopt,
                                      fs::path(std::move(Path)));
      }))
      .def(py::init(
          [](std::string Path,
             std::optional<vf::FeatureSourceRange::FeatureSourceLocation>
                 Start) {
            return vf::FeatureSourceRange(Start, std::nullopt,
                                          fs::path(std::move(Path)));
          }))
      .def(py::init(
          [](std::string Path,
             std::optional<vf::FeatureSourceRange::FeatureSourceLocation> Start,
             std::optional<vf::FeatureSourceRange::FeatureSourceLocation> End) {
            return vf::FeatureSourceRange(Start, End,
                                          fs::path(std::move(Path)));
          }))
      .def_property(
          "path",
          [](vf::FeatureSourceRange &Loc) { return Loc.getPath().string(); },
          &vf::FeatureSourceRange::setPath,
          R"pbdoc(Path to the source file)pbdoc")
      .def_property_readonly(
          "start", &vf::FeatureSourceRange::getStart,
          R"pbdoc(Get the start `LineColumnOffset` of this `Location`.)pbdoc")
      .def_property_readonly(
          "end", &vf::FeatureSourceRange::getEnd,
          R"pbdoc(Get the end `LineColumnOffset` of this `Location`.)pbdoc")
      .def("__str__", &vf::FeatureSourceRange::toString)
      .def(
          "__eq__",
          [](const vf::FeatureSourceRange &Self,
             const vf::FeatureSourceRange *Other) {
            if (!Other) {
              return false;
            }
            return Self == *Other;
          },
          py::arg("Other").none(true));

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
      .def(
          "__eq__",
          [](const vf::FeatureSourceRange::FeatureSourceLocation &Self,
             const vf::FeatureSourceRange::FeatureSourceLocation *Other) {
            if (!Other) {
              return false;
            }
            return Self == *Other;
          },
          py::arg("Other").none(true));
}

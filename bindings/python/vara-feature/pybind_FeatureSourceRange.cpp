#include "pybind_common.h"
#include "vara/Feature/FeatureSourceRange.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace vf = vara::feature;
namespace py = pybind11;

template <typename T>
std::optional<T> convertOptional(const std::optional<T> &Value) {
  if (Value.has_value()) {
    return {std::move(Value.value())};
  }
  return std::nullopt;
}

void init_feature_location_module(py::module &M) {
  py::class_<vf::FeatureSourceRange> Loc(M, "Location");
  Loc.def(py::init([](std::string Path) {
       return vf::FeatureSourceRange(fs::path(std::move(Path)));
     }))
      .def(py::init(
          [](const std::string &Path,
             const std::optional<vf::FeatureSourceRange::FeatureSourceLocation>
                 &Start) {
            return vf::FeatureSourceRange(fs::path(Path),
                                          convertOptional(Start));
          }))
      .def(py::init(
          [](const std::string &Path,
             const std::optional<vf::FeatureSourceRange::FeatureSourceLocation>
                 &Start,
             const std::optional<vf::FeatureSourceRange::FeatureSourceLocation>
                 &End) {
            return vf::FeatureSourceRange(
                fs::path(Path), convertOptional(Start), convertOptional(End),
                vf::FeatureSourceRange::Category::necessary);
          }))
      .def(py::init(
          [](const std::string &Path,
             const std::optional<vf::FeatureSourceRange::FeatureSourceLocation>
                 &Start,
             const std::optional<vf::FeatureSourceRange::FeatureSourceLocation>
                 &End,
             vf::FeatureSourceRange::Category Category) {
            return vf::FeatureSourceRange(fs::path(Path),
                                          convertOptional(Start),
                                          convertOptional(End), Category);
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
      .def_property("category", &vf::FeatureSourceRange::getCategory,
                    &vf::FeatureSourceRange::setCategory,
                    R"pbdoc(Category of Location)pbdoc")
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
  py::enum_<vf::FeatureSourceRange::Category>(Loc, "Category")
      .value("necessary", vf::FeatureSourceRange::Category::necessary)
      .value("inessential", vf::FeatureSourceRange::Category::inessential);

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

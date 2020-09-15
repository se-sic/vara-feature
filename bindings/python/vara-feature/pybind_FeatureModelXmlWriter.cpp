#include "pybind_common.h"
#include "vara/Feature/FeatureModelWriter.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <iostream>

namespace vf = vara::feature;
namespace py = pybind11;

void init_xml_writer(py::module &M) {
  py::class_<vf::FeatureModelXmlWriter>(M, "FeatureModelXmlWriter")
      .def(py::init<const vf::FeatureModel &>())
      .def(
          "get_feature_model_as_string",
          [](vf::FeatureModelXmlWriter &Fmxw) {
            return Fmxw.writeFeatureModel();
          },
          R"pbdoc(Return the xml representation as string)pbdoc")
      .def(
          "write_feature_model_to_file",
          [](vf::FeatureModelXmlWriter &Fmxw, const std::string StrS) {
            return Fmxw.writeFeatureModel(StrS);
          },
          R"pbdoc(Write feature model to the given file)pbdoc");
}

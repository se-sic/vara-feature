#include "pybind_common.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

namespace py = pybind11;

void init_llvm(py::module &M);
void init_feature_module(py::module &M);
void init_feature_module_location(py::module &M);
void init_feature_model_module(py::module &M);
void init_xml_parser(py::module &M);

PYBIND11_MODULE(vara_feature, M) {
  auto LLVMModule = M.def_submodule("llvm_util");
  init_llvm(M);
  auto FeatureModule = M.def_submodule("feature");
  init_feature_module(FeatureModule);
  init_feature_module_location(FeatureModule);
  auto FeatureModelModule = M.def_submodule("feature_model");
  init_feature_model_module(FeatureModelModule);
  auto XmlParserModule = M.def_submodule("xml_parser");
  init_xml_parser(XmlParserModule);
}

#include "pybind_common.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

namespace py = pybind11;

void init_llvm(py::module &M);
void init_feature_module(py::module &M);
void init_constraint_module(py::module &M);
void init_feature_location_module(py::module &M);
void init_feature_model_module(py::module &M);
void init_feature_model_builder_module(py::module &M);
void init_xml_writer(py::module &M);
void init_configuration_module(py::module &M);

PYBIND11_MODULE(vara_feature, M) {
  auto LLVMModule = M.def_submodule("llvm_util");
  init_llvm(M);
  auto FeatureModule = M.def_submodule("feature");
  init_feature_module(FeatureModule);
  init_feature_location_module(FeatureModule);
  auto ConstraintModule = M.def_submodule("constraint");
  init_constraint_module(ConstraintModule);
  auto FeatureModelModule = M.def_submodule("feature_model");
  init_feature_model_module(FeatureModelModule);
  auto FMWriterModule = M.def_submodule("fm_writer");
  init_xml_writer(FMWriterModule);
  auto FeatureModelBuilderModule = M.def_submodule("feature_model_builder");
  init_feature_model_builder_module(FeatureModelBuilderModule);
  auto ConfigurationModule = M.def_submodule("configuration");
  init_configuration_module(ConfigurationModule);
}

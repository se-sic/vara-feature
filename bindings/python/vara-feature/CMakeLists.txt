pybind11_add_module(
  vara_feature
  pybind_Init.cpp
  pybind_Configuration.cpp
  pybind_Constraint.cpp
  pybind_Feature.cpp
  pybind_FeatureModel.cpp
  pybind_FeatureModelBuilder.cpp
  pybind_FeatureModelXmlWriter.cpp
  pybind_FeatureSourceRange.cpp
  pybind_llvm.cpp
)

set(LLVM_LINK_COMPONENTS Core Support)

target_link_libraries(vara_feature LINK_PUBLIC VaRAFeature VaRASolver ${STD_FS_LIB})

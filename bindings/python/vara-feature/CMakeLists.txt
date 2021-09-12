pybind11_add_module(vara_feature
  pybind_Init.cpp
  pybind_Feature.cpp
  pybind_FeatureModel.cpp
  pybind_FeatureModelXmlWriter.cpp
  pybind_FeatureSourceRange.cpp
  pybind_llvm.cpp
)

target_link_libraries(vara_feature
  LINK_PUBLIC
  VaRAFeature
  stdc++fs
)

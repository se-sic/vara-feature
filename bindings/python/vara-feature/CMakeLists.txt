find_library(VaRASolver_LIB
  NAMES VaRASolver
  PATHS /home/michael/Documents/vara-feature/build/temp.linux-x86_64-cpython-312/lib/Solver
  REQUIRED
  NO_DEFAULT_PATH
)

if(NOT VaRASolver_LIB)
  message(FATAL_ERROR "VaRASolver library not found!")
endif()

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

target_link_libraries(
  vara_feature
  LINK_PUBLIC
  VaRAFeature
  ${VaRASolver_LIB}  
  ${STD_FS_LIB}
)
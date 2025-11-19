add_library(bdd STATIC
  src/BDDFactory.cpp
  src/Constraints.cpp       # add this if it implements your visitor/helpers
  src/BDDFeats.cpp          # add if it exists
  src/Probabilities.cpp     # add if it exists
)

# Public include path so dependents can `#include "BDDFactory.h"`
target_include_directories(bdd
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/external/oxidd/include 
)

# If you need to link against OxiDD C FFI or VaRA libs, do it here:
# (adjust paths/targets to your tree)
find_package(LibXml2 REQUIRED)

target_link_libraries(bdd
  PUBLIC 
    oxidd_ffi_c
  PRIVATE
    LibXml2::LibXml2
    VaRAFeature      # if BDD uses these symbols
    VaRASampling     # if needed
)

target_compile_features(bdd PUBLIC cxx_std_20)
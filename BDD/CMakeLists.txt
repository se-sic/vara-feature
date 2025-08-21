add_library(bdd STATIC
  src/BDDFactory.cpp
  src/Constraints.cpp       # add this if it implements your visitor/helpers
  src/Relationships.cpp     # add if it exists
  src/BDDFeats.cpp          # add if it exists
  src/Probabilities.cpp     # add if it exists
)

# Public include path so dependents can `#include "BDDFactory.h"`
target_include_directories(bdd
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# OxiDD headers so BDDFactory.cpp can `#include "oxidd/capi.h"`
target_include_directories(bdd
  PRIVATE
    ${CMAKE_SOURCE_DIR}/external/oxidd/bindings/cpp/include
)

# If you need to link against OxiDD C FFI or VaRA libs, do it here:
# (adjust paths/targets to your tree)
find_package(LibXml2 REQUIRED)

target_link_libraries(bdd
  PRIVATE
    LibXml2::LibXml2
    oxidd_ffi_c      # if the OxiDD C library is needed; ensure link dirs set at root
    VaRAFeature      # if BDD uses these symbols
    VaRASampling     # if needed
)

target_compile_features(bdd PUBLIC cxx_std_20)
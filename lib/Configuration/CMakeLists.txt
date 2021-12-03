set(CONFIGURATION_LIB_SRC
  Configuration.cpp
)

set(LLVM_LINK_COMPONENTS
  Support
  Demangle
  Core
)

add_vara_library(VaRAConfiguration
  ${CONFIGURATION_LIB_SRC}
)


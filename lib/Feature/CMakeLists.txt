set(FEATURE_LIB_SRC
  Feature.cpp
)

set(LLVM_LINK_COMPONENTS
  Support
  Demangle
  Core
)

add_vara_library(VaRAFeature
  ${FEATURE_LIB_SRC}
)

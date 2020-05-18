set(FEATURE_LIB_SRC
  Feature.cpp
  FeatureModelParser.cpp
)

set(LLVM_LINK_COMPONENTS
  Support
  Demangle
  Core
)

add_vara_library(VaRAFeature
  ${FEATURE_LIB_SRC}
)

target_link_libraries(VaRAFeature
  LINK_PUBLIC
  ${LIBXML2_LIBRARIES}
  stdc++fs
)

set(FEATURE_LIB_SRC
  Constraint.cpp
  Feature.cpp
  FeatureModel.cpp
  FeatureModelParser.cpp
  FeatureModelTransaction.cpp
  FeatureModelWriter.cpp
  OrderedFeatureVector.cpp
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
  )

if (HAS_STD_EXPERIMENTAL_FILESYSTEM OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(VaRAFeature
    LINK_PUBLIC
    stdc++fs
    )
endif ()


set(SAMPLING_LIB_SRC
  SamplingMethods.cpp
  SampleSetParser.cpp
)

set(LLVM_LINK_COMPONENTS
)

add_vara_library(VaRASampling
  ${SAMPLING_LIB_SRC}
)

target_link_libraries(VaRASampling
  VaRAConfiguration
  csv
)

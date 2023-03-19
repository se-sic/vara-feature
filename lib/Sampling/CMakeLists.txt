set(SAMPLING_LIB_SRC
  SamplingMethods.cpp
  SampleSetParser.cpp
  SampleSetWriter.cpp
)

set(LLVM_LINK_COMPONENTS
  Core
  Support
)

add_vara_library(VaRASampling
  ${SAMPLING_LIB_SRC}
)

target_link_libraries(VaRASampling
  LINK_PUBLIC
  VaRAConfiguration
  csv
)

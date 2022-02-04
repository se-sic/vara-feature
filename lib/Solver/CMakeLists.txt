set(SAMPLING_LIB_SRC
  SamplingMethods.cpp
)

set(LLVM_LINK_COMPONENTS
)

add_vara_library(VaRASolver
  ${SAMPLING_LIB_SRC}
)

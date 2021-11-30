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

if (HAS_STD_EXPERIMENTAL_FILESYSTEM OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(VaRAConfiguration
    LINK_PUBLIC
    stdc++fs
    )
endif ()


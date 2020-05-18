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
  )

include(CheckCXXSourceCompiles)

CHECK_CXX_SOURCE_COMPILES("#include <filesystem>
  namespace fs = std::filesystem;
  int main () { return 0; }"
  HAS_STD_FILESYSTEM
  )
if (NOT HAS_STD_FILESYSTEM)
  CHECK_CXX_SOURCE_COMPILES("#include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
    int main () { return 0; }"
    HAS_STD_EXPERIMENTAL_FILESYSTEM
    )
  if (HAS_STD_EXPERIMENTAL_FILESYSTEM)
    message(STATUS "Using std::experimental::filesystem")
    add_compile_definitions(STD_EXPERIMENTAL_FILESYSTEM)
    target_link_libraries(VaRAFeature
      LINK_PUBLIC
      stdc++fs
      )
  else ()
    message(FATAL_ERROR "missing filesystem")
  endif ()
else ()
  message(STATUS "Using std::filesystem")
endif ()

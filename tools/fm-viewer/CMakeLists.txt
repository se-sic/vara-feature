set(LLVM_LINK_COMPONENTS Support Demangle Core)

add_vara_executable(fm-viewer FeatureModelViewer.cpp)

target_link_libraries(fm-viewer LINK_PRIVATE VaRAFeature ${STD_FS_LIB})

add_custom_target(
  check-vara-fm-viewer
  COMMAND fm-viewer xml/test.xml -viewer cat | dot | grep . -q
  COMMENT "Target to run fm-viewer tests"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../unittests/resources
)

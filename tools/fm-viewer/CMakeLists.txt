set(LLVM_LINK_COMPONENTS
  Support
  Demangle
  Core
  )

add_vara_executable(fm-viewer
  FeatureModelViewer.cpp
  )

target_link_libraries(fm-viewer
  LINK_PRIVATE
  VaRAFeature
  stdc++fs
  )

add_custom_target(check-vara-fm-viewer
  COMMAND fm-viewer test.xml -viewer cat | dot | grep . -q
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../unittests/resources
  )

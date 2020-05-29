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
  )

add_custom_target(check-vara-fm-viewer
  COMMAND fm-viewer test.xml -viewer dot
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../test
  )

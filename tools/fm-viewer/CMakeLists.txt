set(LLVM_LINK_COMPONENTS
)

add_vara_executable(fm-viewer
  FeatureModelViewer.cpp
)
target_link_libraries(fm-viewer
  LINK_PRIVATE
  VaRAFeature
)

set(LLVM_LINK_COMPONENTS Support Demangle Core)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
find_package(
  QT
  NAMES
  Qt6
  Qt5
  REQUIRED
  COMPONENTS Widgets
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
if(QT_VERSION_MAJOR GREATER 5 OR QT_VERSION_MINOR GREATER_EQUAL 15)
  # add_compile_definitions(QT_HAS_SETPLACEHOLDERTEXT)
endif()
include_directories(../../external/qsourcehighlite)
include_directories(.)

add_vara_executable(
  fm-editor
  main.cpp
  graph/FeatureModelGraph.h
  graph/FeatureModelGraph.cpp
  graph/FeatureNode.h
  graph/FeatureNode.cpp
  graph/FeatureEdge.h
  graph/FeatureEdge.cpp
  tree/FeatureTreeViewModel.h
  tree/FeatureTreeViewModel.cpp
  tree/FeatureTreeItem.h
  tree/FeatureTreeItem.cpp
  FeatureModelEditor.h
  FeatureModelEditor.cpp
  FeatureAddDialog.h
  FeatureAddDialog.cpp
  ../../external/qsourcehighlite/qsourcehighliter.h
  ../../external/qsourcehighlite/qsourcehighliter.cpp
  ../../external/qsourcehighlite/languagedata.h
  ../../external/qsourcehighlite/languagedata.cpp
  ../../external/qsourcehighlite/qsourcehighliterthemes.h
  ../../external/qsourcehighlite/qsourcehighliterthemes.cpp
)
target_link_libraries(
  fm-editor
  LINK_PRIVATE
  VaRAFeature
  Qt${QT_VERSION_MAJOR}::Widgets
  ${STD_FS_LIB}
)

add_custom_target(
  check-vara-fm-editor
  COMMAND fm-editor xml/test.xml -viewer cat | dot | grep . -q
  COMMENT "Unittests"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../unittests/resources
)

file(
  WRITE "${CMAKE_CURRENT_BINARY_DIR}/fm-editor_autogen/.clang-tidy"
  "
---
Checks: '-*,llvm-twine-local'
...
"
)

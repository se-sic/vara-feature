set(LLVM_LINK_COMPONENTS
        Support
        Demangle
        Core
)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 COMPONENTS Widgets)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)
endif()
include_directories(../../external/qsourcehighlite)
include_directories(.)

add_vara_executable(fm-editor
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
        FeatureModelEditor.h FeatureModelEditor.cpp
        FeatureAddDialog.h FeatureAddDialog.cpp
        ../../external/qsourcehighlite/qsourcehighliter.h
        ../../external/qsourcehighlite/qsourcehighliter.cpp
        ../../external/qsourcehighlite/languagedata.h
        ../../external/qsourcehighlite/languagedata.cpp
        ../../external/qsourcehighlite/qsourcehighliterthemes.h
        ../../external/qsourcehighlite/qsourcehighliterthemes.cpp
)
target_link_libraries(fm-editor
        LINK_PRIVATE
        VaRAFeature
        Qt::Widgets
        ${STD_FS_LIB}
)

add_custom_target(check-vara-fm-editor
        COMMAND fm-editor xml/test.xml -viewer cat | dot | grep . -q
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../unittests/resources
)

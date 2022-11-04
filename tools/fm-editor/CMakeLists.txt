set(LLVM_LINK_COMPONENTS
        Support
        Demangle
        Core
        )

add_vara_executable(fm-editor
        main.cpp
        FeatureModelGraph.h
        FeatureModelGraph.cpp
        FeatureNode.h
        FeatureNode.cpp
        FeatureEdge.h
        FeatureEdge.cpp
        )

target_link_libraries(fm-editor
        LINK_PRIVATE
        VaRAFeature
        Qt::Core
        Qt::Gui
        Qt::Widgets
        ${STD_FS_LIB}
        )

add_custom_target(check-vara-fm-editor
        COMMAND fm-editor xml/test.xml -viewer cat | dot | grep . -q
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../unittests/resources
        )

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
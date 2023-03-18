set(LLVM_LINK_COMPONENTS
  Support
  Demangle
  Core
  )

add_vara_executable(config-generator
        ConfigurationGenerator.cpp
  )

target_link_libraries(config-generator
  LINK_PRIVATE
  VaRAFeature
  ${STD_FS_LIB}
  )

add_custom_target(check-vara-fm-viewer
  COMMAND config-generator xml/test.xml -viewer cat | dot | grep . -q
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../unittests/resources
  )

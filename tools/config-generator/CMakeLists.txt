set(LLVM_LINK_COMPONENTS Support Demangle Core)

add_vara_executable(config-generator ConfigurationGenerator.cpp)

target_link_libraries(
  config-generator
  LINK_PRIVATE
  VaRAFeature
  VaRASampling
  VaRASolver
  csv
  ${STD_FS_LIB}
)

add_custom_target(
  check-vara-config-generator
  COMMAND config-generator test_dune_num.xml -type sample -csv configs_dune.csv
          -out configs.yml
  COMMENT "Target to run the configuration generator using csv import."
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../unittests/resources
)

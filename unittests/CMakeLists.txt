set(VARA_FEATURE_TEST_UNITS
  VaRAFeatureUnitTests
  VaRAConfigurationUnitTests
)
set(VARA_FEATURE_LIBS
  VaRAFeature
  VaRAConfiguration
)

foreach(TEST_UNIT ${VARA_FEATURE_TEST_UNITS})
  message(STATUS "Setting: ${TEST_UNIT}")
  add_custom_target(${TEST_UNIT})

  set_target_properties(${TEST_UNIT} PROPERTIES FOLDER
    "VaRA/vara-feature")
endforeach()

if (VARA_FEATURE_IN_TREE)
  # Run only unit tests
  add_lit_testsuite(check-vara-feature-unittests
    "Running vara-feature unit tests only"
    ${CMAKE_CURRENT_BINARY_DIR}
    PARAMS vara_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
    DEPENDS ${VARA_FEATURE_TEST_UNITS}
    )
  set_target_properties(check-vara-feature-unittests PROPERTIES FOLDER "VaRA")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
    ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )
else()
  add_custom_target(check-vara-feature-unittests
    COMMAND ${CMAKE_CTEST_COMMAND} --progress --output-on-failure -j 8
    DEPENDS ${VARA_FEATURE_TEST_UNITS}
  )
endif()

# add_vara_unittest(test_dirname file1.cpp file2.cpp)
#
# Will compile the list of files together and link against VaRA and its dependences.
function(add_vara_unittest test_name)
  if(COMMAND add_unittest)
    foreach (TEST_UNIT ${VARA_FEATURE_TEST_UNITS})
      add_unittest(${TEST_UNIT} ${test_name} ${ARGN})
    endforeach()
  else()
    add_executable(${test_name} EXCLUDE_FROM_ALL ${ARGN})
    set_target_properties(${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    target_link_libraries(${test_name} PRIVATE gtest_main gtest stdc++fs)
    foreach(TEST_UNIT ${VARA_FEATURE_TEST_UNITS})
      add_dependencies(${TEST_UNIT} ${test_name})
    endforeach()

    set_property(TARGET ${test_name} PROPERTY FOLDER "VaRA/vara-feature")
    add_test(
      NAME "${test_name}"
      COMMAND ${test_name} ${CATCH_TEST_FILTER}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set(CTEST_OUTPUT_ON_FAILURE 1)
  endif()
  target_link_libraries(${test_name} PRIVATE ${VARA_FEATURE_LIBS})
endfunction()

# Adds helper headers
include_directories(
  .
)

add_subdirectory(Configuration)
add_subdirectory(Feature)
add_subdirectory(Utils)
add_subdirectory(resources)

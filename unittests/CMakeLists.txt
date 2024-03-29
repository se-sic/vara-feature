set(VARA_FEATURE_TEST_UNITS VaRAFeatureUnitTests VaRAConfigurationUnitTests
                            VaRASamplingUnitTests VaRASolverUnitTests
)
set(VARA_FEATURE_LIBS VaRAFeature VaRAConfiguration VaRASampling VaRASolver)
set(VARA_FEAUTRE_TEST_WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})

foreach(test_unit ${VARA_FEATURE_TEST_UNITS})
  message(STATUS "Setting: ${test_unit}")
  add_custom_target(${test_unit} COMMENT "Target to run a specific test unit")

  set_target_properties(${TEST_UNIT} PROPERTIES FOLDER "VaRA/vara-feature")
endforeach()

if(VARA_FEATURE_IN_TREE)
  # Run only unit tests
  add_lit_testsuite(
    check-vara-feature-unittests
    "Running vara-feature unit tests only"
    ${CMAKE_CURRENT_BINARY_DIR}
    PARAMS
    vara_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
    DEPENDS
    ${VARA_FEATURE_TEST_UNITS}
  )
  set_target_properties(check-vara-feature-unittests PROPERTIES FOLDER "VaRA")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
    ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )
else()
  add_custom_target(
    check-vara-feature-unittests
    COMMAND ${CMAKE_CTEST_COMMAND} --progress --output-on-failure -j 8
    COMMENT "Target to run all vara-feature unittests"
    DEPENDS ${VARA_FEATURE_TEST_UNITS}
  )
endif()

# add_vara_unittest(test_dirname file1.cpp file2.cpp)
#
# Will compile the list of files together and link against VaRA and its
# dependences.
function(add_vara_unittest test_unit test_name)
  if(COMMAND add_unittest)
    add_unittest(${test_unit} ${test_name} ${ARGN})
  else()
    add_executable(${test_name} EXCLUDE_FROM_ALL ${ARGN})
    set_target_properties(
      ${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                              ${CMAKE_CURRENT_BINARY_DIR}
    )

    target_link_libraries(${test_name} PRIVATE gtest_main gtest ${STD_FS_LIB})
    add_dependencies(${test_unit} ${test_name})

    set_property(TARGET ${test_name} PROPERTY FOLDER "VaRA/vara-feature")
    add_test(
      NAME "${test_name}"
      COMMAND ${test_name} ${CATCH_TEST_FILTER}
      WORKING_DIRECTORY ${VARA_FEAUTRE_TEST_WORKING_DIR}
    )
    set(CTEST_OUTPUT_ON_FAILURE 1) # cmake-lint: disable=C0103
  endif()
  target_link_libraries(${test_name} PRIVATE ${VARA_FEATURE_LIBS})
endfunction()

# Adds helper headers
include_directories(.)

add_subdirectory(Configuration)
add_subdirectory(Feature)
add_subdirectory(Sampling)
add_subdirectory(Solver)
add_subdirectory(Utils)
add_subdirectory(resources)

add_custom_target(
  check-vara-feature-python
  COMMAND
    ${CMAKE_COMMAND} -E env
    PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/../vara-feature ${PYTHON_EXECUTABLE}
    -m unittest discover
  DEPENDS vara_feature
  COMMENT "Meta target to run all binding tests"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
)

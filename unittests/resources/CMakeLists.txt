macro(add_test_file file_name)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file_name}
    ${CMAKE_CURRENT_BINARY_DIR}/
    COPYONLY
  )
endmacro()

set(FEATURE_LIB_TEST_FILES
  test.xml
  test_children.xml
  test_excludes.xml
  test_out.xml
  )

foreach(file_name ${FEATURE_LIB_TEST_FILES})
  add_test_file(${file_name})
endforeach()

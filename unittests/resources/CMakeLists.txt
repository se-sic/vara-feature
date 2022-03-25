macro(add_test_file file_name)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file_name}
    ${CMAKE_CURRENT_BINARY_DIR}/
    COPYONLY
  )
endmacro()

set(FEATURE_LIB_TEST_FILES
  sxfm/sxfm_example.sxfm
  sxfm/test.sxfm
  sxfm/test_wrong_indentation.sxfm
  sxfm/test_wrong_xml_format.sxfm
  sxfm/test_wrong_constraint_format.sxfm
  xml/error_mismatch_parent_child.xml
  xml/error_missing_child.xml
  xml/error_missing_exclude.xml
  xml/error_missing_implication.xml
  xml/error_missing_parent.xml
  xml/error_root_root.xml
  xml/test.xml
  xml/test_children.xml
  xml/test_excludes.xml
  xml/test_member_offset.xml
  xml/test_numbers.xml
  xml/test_only_children.xml
  xml/test_only_parents.xml
  xml/test_out_of_order.xml
  xml/test_with_whitespaces.xml
  )

foreach(file_name ${FEATURE_LIB_TEST_FILES})
  add_test_file(${file_name})
endforeach()

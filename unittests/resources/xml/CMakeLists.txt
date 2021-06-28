set(FEATURE_LIB_TEST_XML_FILES
  test.xml
  test_children.xml
  test_excludes.xml
  test_mismatch_parent_child.xml
  test_non_existent_child.xml
  test_non_existent_exclude.xml
  test_non_existent_implication.xml
  test_non_existent_parent.xml
  test_only_children.xml
  test_only_parents.xml
  test_out_of_order.xml
  test_root_empty_parent.xml
  test_root_no_parent.xml
  test_root_parent_root.xml
  )

foreach (file_name ${FEATURE_LIB_TEST_XML_FILES})
  add_test_file(${file_name})
endforeach ()

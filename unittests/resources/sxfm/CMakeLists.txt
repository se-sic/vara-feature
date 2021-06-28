set(FEATURE_LIB_TEST_SXFM_FILES
  sxfm_example.sxfm
  test.sxfm
  test_wrong_indentation.sxfm
  test_wrong_xml_format.sxfm
  )

foreach (file_name ${FEATURE_LIB_TEST_SXFM_FILES})
  add_test_file(${file_name})
endforeach ()

macro(add_test_file file_name)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file_name}
    ${CMAKE_CURRENT_BINARY_DIR}/
    COPYONLY
  )
endmacro()

add_subdirectory(sxfm)
add_subdirectory(xml)

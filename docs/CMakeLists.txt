find_package(Doxygen REQUIRED)

set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR})
set(DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/docs)

set(DOXYGEN_HHC_PROGRAM)
set(DOXYGEN_GENERATE_HTMLHELP NO)
set(DOXYGEN_ENABLE_SEARCHENGINE)

option(VARA_FEATURE_DOXYGEN_SVG
       "Use svg instead of png files for doxygen graphs." OFF
)
if(VARA_DOXYGEN_SVG)
  set(DOT_IMAGE_FORMAT "svg")
else()
  set(DOT_IMAGE_FORMAT "png")
endif()

set(DOT)

set(DOXYGEN_CONFIGURED_INPUT ${DOXYGEN_OUTPUT_DIR}/doxyfile)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/doxygen.cfg.in ${DOXYGEN_CONFIGURED_INPUT} @ONLY
)

add_custom_target(
  build-docs
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIGURED_INPUT}
  COMMENT "Target to build documentation"
  WORKING_DIRECTORY ${DOXYGEN_INPUT_DIR}
)

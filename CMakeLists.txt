if (DEFINED LLVM_MAIN_SRC_DIR)
  set(VARA_FEATURE_IN_TREE 1)
endif()

if (NOT VARA_FEATURE_IN_TREE)
  cmake_minimum_required (VERSION 3.12)
  project(vara-feature)

  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# CMake CXX options
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake options
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

# VARA_FEATURE options
option(VARA_FEATURE_COLORED_OUTPUT "Produce ANSI-colored output" TRUE)
if (${VARA_FEATURE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()

find_program(VARA_FEATURE_RUN_CLANG_TIDY
  NAMES run-clang-tidy.py
  PATHS /usr/lib/llvm/*/share/clang/
)
message(STATUS ${CMAKE_BINARY_DIR})
add_custom_target(tidy-vara-feature
  ${VARA_FEATURE_RUN_CLANG_TIDY}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Run clang-tidy over vara-feature files"
)

# Setup the program
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include("vara_feature_macros")

# Include external dependencies
if (NOT VARA_FEATURE_IN_TREE)
  set(MIN_LLVM_REQUIRED 9)
  SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
  SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

  # Only search for LLVM if we build out of tree
  find_package(LLVM REQUIRED CONFIG)
  if (NOT "${LLVM_VERSION_MAJOR}" GREATER_EQUAL "${MIN_LLVM_REQUIRED}")
    message(FATAL_ERROR "LLVM version error: minimal required LLVM-${MIN_LLVM_REQUIRED} but found LLVM-${LLVM_VERSION_MAJOR}. ")
  endif()
  find_library(LLVM_LIBRARY NAMES llvm REQUIRED)
  include_directories(${LLVM_INCLUDE_DIRS})
  link_directories(${LLVM_LIB_PATH} ${LLVM_LIBRARY_DIRS})
  add_definitions(${LLVM_DEFINITIONS})

  set(USE_SHARED ON)
endif()

include_directories(include/)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(tools)

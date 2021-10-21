if (DEFINED LLVM_MAIN_SRC_DIR)
  set(VARA_FEATURE_IN_TREE 1)
endif ()

if (NOT VARA_FEATURE_IN_TREE)
  cmake_minimum_required(VERSION 3.12)
  project(vara-feature)

  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif ()

# CMake CXX options
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake options
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-stdlib=libstdc++)
endif ()

# VARA_FEATURE options
option(VARA_FEATURE_COLORED_OUTPUT "Produce ANSI-colored output" TRUE)
if (${VARA_FEATURE_COLORED_OUTPUT})
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
  endif ()
endif ()

option(VARA_FEATURE_SANITIZE "Build vara-features with ASan and UBSan" FALSE)
if (${VARA_FEATURE_SANITIZE})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
endif ()

option(VARA_FEATURE_BUILD_PYTHON_BINDINGS "Build vara-features python bindings" TRUE)
if (${VARA_FEATURE_BUILD_PYTHON_BINDINGS})
  if (${VARA_FEATURE_SANITIZE})
    message(FATAL_ERROR "Cannot build python bindings together with sanitizers.")
  endif ()
  add_compile_options(-fPIC)
endif ()

option(VARA_FEATURE_BUILD_DOC "Generate documentation" FALSE)

# Extra debug warning flags
string(TOUPPER ${CMAKE_BUILD_TYPE} build_type)
if (build_type MATCHES "^(DEBUG)")
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif ()

# Enable coverage tracking
option(VARA_FEATURE_ENABLE_COVERAGE "Enable code coverage tracking" FALSE)
if (VARA_FEATURE_ENABLE_COVERAGE)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(--coverage -O0)
    add_link_options(--coverage)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/profiles" VARA_FEAUTRE_PROFILE_DATA_DIR)
    set(VARA_FEATURE_PROFILE_PATTERN
        "${VARA_FEAUTRE_PROFILE_DATA_DIR}/profile-%p.profraw"
    )

    add_compile_options(-fprofile-instr-generate=${VARA_FEATURE_PROFILE_PATTERN} -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate=${VARA_FEATURE_PROFILE_PATTERN})
  endif()
endif()

# Setup git submodules (external projects)
find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  option(VARA_FEATURE_GIT_SUBMODULE "Checkout vara-feature's submodules during build" ON)
  if (VARA_GIT_SUBMODULE)
    message(STATUS "vara-feature update submodules")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT
    )
    if (NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(
        FATAL_ERROR "git submodule update --init failed with ${VARA_FEATURE_GIT_SUBMODULE}"
      )
    endif ()
  endif ()
endif ()

# Setup custom targets

find_program(VARA_FEATURE_RUN_CLANG_TIDY
  NAMES run-clang-tidy-12.py run-clang-tidy-12 run-clang-tidy.py run-clang-tidy clang-tidy-12 clang-tidy
  PATHS /usr/lib/llvm/*/share/clang/ /usr/bin/
  )

add_custom_target(tidy-vara-feature
  ${VARA_FEATURE_RUN_CLANG_TIDY} -header-filter=vara/.* lib/* unittests/* tools/*
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Run clang-tidy over vara-feature files"
  )

# Setup the program
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include("vara_feature_macros")

# Include external dependencies
if (NOT VARA_FEATURE_IN_TREE)
  set(MIN_LLVM_REQUIRED 10)

  # Only search for LLVM if we build out of tree
  if (DEFINED LLVM_REQUESTED_VERSION)
    message(STATUS "Using LLVM ${LLVM_REQUESTED_VERSION}")
    find_package(LLVM ${LLVM_REQUESTED_VERSION} REQUIRED CONFIG)
  else ()
    message(STATUS "Using LLVM (default)")
    set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
    find_package(LLVM REQUIRED CONFIG)
  endif ()

  if (NOT "${LLVM_VERSION_MAJOR}" GREATER_EQUAL "${MIN_LLVM_REQUIRED}")
    message(FATAL_ERROR "LLVM version error: minimal required LLVM-${MIN_LLVM_REQUIRED} but found LLVM-${LLVM_VERSION_MAJOR}. ")
  endif ()
  include_directories(${LLVM_INCLUDE_DIRS})
  link_directories(${LLVM_LIB_PATH} ${LLVM_LIBRARY_DIRS})
  add_definitions(${LLVM_DEFINITIONS})

  set(USE_SHARED ON)
endif ()

include(CheckCXXSourceCompiles)

# Check whether std::filesystem or std::experimental::filesystem can be included
check_std_filesystems(VARA_HAS_STD_FILESYSTEM)
if (NOT VARA_HAS_STD_FILESYSTEM)
  CHECK_CXX_SOURCE_COMPILES("#include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
    int main () { return 0; }"
    HAS_STD_EXPERIMENTAL_FILESYSTEM
    )
  if (HAS_STD_EXPERIMENTAL_FILESYSTEM)
    message(STATUS "Using std::experimental::filesystem")
    add_compile_definitions(STD_EXPERIMENTAL_FILESYSTEM)
  else ()
    message(FATAL_ERROR "missing filesystem")
  endif ()
else ()
  message(STATUS "Using std::filesystem")
endif ()

include_directories(include/)

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(tools)

if (${VARA_FEATURE_BUILD_PYTHON_BINDINGS})
  find_package(Python COMPONENTS Interpreter Development REQUIRED)
  add_subdirectory(external/pybind11)
  add_subdirectory(bindings)
endif ()

if (${VARA_FEATURE_BUILD_DOC})
  add_subdirectory(docs)
endif()

if (VARA_FEATURE_IN_TREE)
  # Set llvm distributed includes for gtest header
  include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
  include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googlemock/include)
else ()
  add_subdirectory(external/googletest EXCLUDE_FROM_ALL)
  include_directories(external/googletest/googletest/include)
  include_directories(external/googletest/googlemock/include)

  enable_testing()
endif ()

add_subdirectory(unittests)
add_subdirectory(scripts)

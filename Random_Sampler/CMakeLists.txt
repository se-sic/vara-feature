cmake_minimum_required(VERSION 3.15)
project(Random_Sampler)

find_package(TinyXML2 REQUIRED)
if(NOT TinyXML2_FOUND)
  find_library(TINYXML2_LIB NAMES tinyxml2
    PATHS /usr/lib/x86_64-linux-gnu
    NO_DEFAULT_PATH)
  find_path(TINYXML2_INCLUDE_DIR NAMES tinyxml2.h
    PATHS /usr/include
    NO_DEFAULT_PATH)
  
  if(TINYXML2_LIB AND TINYXML2_INCLUDE_DIR)
    add_library(TinyXML2::TinyXML2 UNKNOWN IMPORTED)
    set_target_properties(TinyXML2::TinyXML2 PROPERTIES
      IMPORTED_LOCATION "${TINYXML2_LIB}"
      INTERFACE_INCLUDE_DIRECTORIES "${TINYXML2_INCLUDE_DIR}")
  else()
    message(FATAL_ERROR "TinyXML2 not found. Install with: sudo apt-get install libtinyxml2-dev")
  endif()
endif()

#if(APPLE)
#  set(OXIDD_LIB_EXT "dylib")
#else()
#  set(OXIDD_LIB_EXT "so")
#endif()
#set(OXIDD_LIB_NAME "liboxidd_ffi_c.${OXIDD_LIB_EXT}")

#foreach(OXIDD_SEARCH_PATH 
#  "${CMAKE_SOURCE_DIR}/build/external/oxidd"
#  "${CMAKE_SOURCE_DIR}/external/oxidd/lib"
#  "/usr/local/lib"
#)
#  if(EXISTS "${OXIDD_SEARCH_PATH}/${OXIDD_LIB_NAME}")
#    set(OXIDD_LIB_PATH "${OXIDD_SEARCH_PATH}/${OXIDD_LIB_NAME}")
#    break()
#  endif()
#endforeach()

add_library(coverage_lib
  src/CoverageIntegration.cpp
  src/CoverageMetrics.cpp
  src/BDDSampler.cpp
)

add_executable(random_sampler
  src/main.cpp
  src/BDDSampler.cpp
  src/Plotter.cpp
  src/CoverageIntegration.cpp
  src/CoverageMetrics.cpp
  #../BDD/src/BDDFactory.cpp
)

# Includes
target_include_directories(coverage_lib PUBLIC
  ${CMAKE_SOURCE_DIR}/Random_Sampler/include
  ${CMAKE_SOURCE_DIR}/include/vara/Solver
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../BDD/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/external/oxidd/bindings/cpp/include
  ${CMAKE_BINARY_DIR}/external/oxidd/include
  ${CMAKE_BINARY_DIR}/_deps/tl-expected-src/include
  /opt/homebrew/Cellar/tinyxml2/11.0.0/include
)

# 2. Configuration for the executable
target_include_directories(random_sampler PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/include/vara/Solver
  ${CMAKE_CURRENT_SOURCE_DIR}/../BDD/include
  ${CMAKE_SOURCE_DIR}/external/oxidd/bindings/cpp/include
  ${CMAKE_BINARY_DIR}/_deps/tl-expected-src/include
)

# Libraries
find_package(tinyxml2 REQUIRED)

target_link_libraries(random_sampler PRIVATE
  coverage_lib PRIVATE
  bdd
  VaRAFeature
  VaRASampling
  VaRAConfiguration
  tinyxml2::tinyxml2
  ${LLVM_LIBS}
  #VaRAFeature
  #/opt/homebrew/Cellar/tinyxml2/11.0.0/lib/libtinyxml2.dylib
  #${CMAKE_SOURCE_DIR}/build/external/oxidd/liboxidd_ffi_c.dylib
)

#target_link_directories(random_sampler PRIVATE
#  ${CMAKE_SOURCE_DIR}/build/external/oxidd
#)

add_executable(compute_constraint_ratio
    src/compute_constraint_ratio.cpp
)

target_link_libraries(compute_constraint_ratio
    PRIVATE
    VaRAFeature                         
    oxidd                                
    ${CMAKE_BINARY_DIR}/external/oxidd/liboxidd_ffi_c.a   
    ${CMAKE_BINARY_DIR}/BDD/libbdd.a                      
    coverage_lib
    ${LLVM_LIBRARIES} 
)

target_sources(random_sampler PRIVATE
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/BDDSampler.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/Plotter.cpp
)

target_sources(compute_constraint_ratio PRIVATE
    ${CMAKE_SOURCE_DIR}/Random_Sampler/tests/utilities/test_helpers.cpp
)

target_include_directories(compute_constraint_ratio PRIVATE
    ${CMAKE_SOURCE_DIR}/BDD/include
    # If the oxidd alias doesn't provide headers, add:
    # ${CMAKE_SOURCE_DIR}/external/oxidd/bindings/cpp/include
)

add_executable(compare_configs
    src/compare_configs.cpp
)

target_link_libraries(compare_configs
    PRIVATE
    VaRAFeature
    oxidd_ffi_c
    bdd
    coverage_lib
)

target_include_directories(compare_configs PRIVATE
    ${CMAKE_SOURCE_DIR}/BDD/include                 
    ${CMAKE_SOURCE_DIR}/external/oxidd/bindings/cpp/include  
)

set_target_properties(random_sampler PROPERTIES
  BUILD_RPATH "${CMAKE_BINARY_DIR}/external/oxidd"
  INSTALL_RPATH_USE_LINK_PATH TRUE
)

option(BUILD_TESTS "Build test suite" OFF)

install(TARGETS compute_constraint_ratio
    RUNTIME DESTINATION bin
)

if(BUILD_TESTS)
    message(STATUS "Building tests...")
    enable_testing()
    add_subdirectory(tests)
endif()
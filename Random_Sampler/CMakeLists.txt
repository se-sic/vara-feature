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

if(APPLE)
  set(OXIDD_LIB_EXT "dylib")
else()
  set(OXIDD_LIB_EXT "so")
endif()
set(OXIDD_LIB_NAME "liboxidd_ffi_c.${OXIDD_LIB_EXT}")

foreach(OXIDD_SEARCH_PATH 
  "${CMAKE_SOURCE_DIR}/build/external/oxidd"
  "${CMAKE_SOURCE_DIR}/external/oxidd/lib"
  "/usr/local/lib"
)
  if(EXISTS "${OXIDD_SEARCH_PATH}/${OXIDD_LIB_NAME}")
    set(OXIDD_LIB_PATH "${OXIDD_SEARCH_PATH}/${OXIDD_LIB_NAME}")
    break()
  endif()
endforeach()

add_executable(random_sampler
  src/main.cpp
  src/BDDSampler.cpp
  src/GraphViz.cpp
  src/FeatureDiagram.cpp
  src/Sampler.cpp
  src/Plotter.cpp
  ../BDD/src/BDDFactory.cpp
)

# Includes
target_include_directories(random_sampler PRIVATE 
  ${CMAKE_SOURCE_DIR}/Random_Sampler/include
  ${CMAKE_SOURCE_DIR}/include/vara/Solver
  /opt/homebrew/Cellar/tinyxml2/11.0.0/include
)

target_include_directories(random_sampler PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../BDD/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Libraries
find_package(tinyxml2 REQUIRED)

target_link_libraries(random_sampler PRIVATE
  bdd
  VaRAFeature
  VaRASampling
  VaRAConfiguration
  oxidd_ffi_c               # from add_subdirectory(external/oxidd)
  tinyxml2::tinyxml2
  ${LLVM_LIBS}
  VaRAFeature
  /opt/homebrew/Cellar/tinyxml2/11.0.0/lib/libtinyxml2.dylib
  ${CMAKE_SOURCE_DIR}/build/external/oxidd/liboxidd_ffi_c.dylib
)

target_link_directories(random_sampler PRIVATE
  ${CMAKE_SOURCE_DIR}/build/external/oxidd
)

target_sources(random_sampler PRIVATE
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/FeatureDiagram.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/GraphViz.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/BDDSampler.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/Sampler.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/Plotter.cpp
)

add_executable(random_sampler
  src/main.cpp
  src/BDDSampler.cpp
  src/GraphViz.cpp
  src/FeatureDiagram.cpp
  src/Z3Helper.cpp
  src/Sampler.cpp
  src/ConfigCounter.cpp
  ../BDD/src/BDDFactory.cpp
)

target_include_directories(random_sampler PRIVATE 
  ${CMAKE_SOURCE_DIR}/Random_Sampler/include
  ${CMAKE_SOURCE_DIR}/external/oxidd/include
  ${CMAKE_SOURCE_DIR}/external/oxidd/bindings/cpp/include
  ${CMAKE_SOURCE_DIR}/include/vara/Solver
  /opt/homebrew/Cellar/tinyxml2/11.0.0/include
)

target_include_directories(random_sampler PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../BDD/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(random_sampler PRIVATE
  bdd
  ${LLVM_LIBS}
  VaRAFeature
  /opt/homebrew/Cellar/tinyxml2/11.0.0/lib/libtinyxml2.dylib
  ${CMAKE_SOURCE_DIR}/build/external/oxidd/liboxidd_ffi_c.dylib
)

target_link_directories(random_sampler PRIVATE
  ${CMAKE_SOURCE_DIR}/build/external/oxidd
)

target_sources(random_sampler PRIVATE
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/ConfigCounter.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/FeatureDiagram.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/GraphViz.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/BDDSampler.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/Z3Helper.cpp
  ${CMAKE_SOURCE_DIR}/Random_Sampler/src/Sampler.cpp
)
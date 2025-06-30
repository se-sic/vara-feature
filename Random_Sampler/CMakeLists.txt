add_executable(random_sampler
    src/main.cpp
    src/ConfigCounter.cpp
    src/Z3Helper.cpp
    src/Sampler.cpp
    src/FeatureDiagram.cpp
    src/GraphViz.cpp
)

target_include_directories(random_sampler PRIVATE 
  ${CMAKE_SOURCE_DIR}/Random_Sampler/include
  ${CMAKE_SOURCE_DIR}/external/oxidd/include
  ${CMAKE_SOURCE_DIR}/external/oxidd/bindings/cpp/include
  ${CMAKE_SOURCE_DIR}/include/vara/Solver
  /opt/homebrew/Cellar/tinyxml2/11.0.0/include
)

target_link_libraries(random_sampler PRIVATE
  ${LLVM_LIBS}
  VaRAFeature
  /opt/homebrew/Cellar/tinyxml2/11.0.0/lib/libtinyxml2.dylib
  ${CMAKE_SOURCE_DIR}/build/external/oxidd/liboxidd_ffi_c.dylib
)
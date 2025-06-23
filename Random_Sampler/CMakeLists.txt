add_executable(random_sampler
    src/main.cpp
    src/ConfigCounter.cpp
    src/Z3Helper.cpp
    src/Sampler.cpp
    src/FeatureDiagram.cpp
    src/GraphViz.cpp
)

target_link_libraries(random_sampler PRIVATE 
${PROJECT_SOURCE_DIR}/include
${LLVM_INCLUDE_DIRS})

target_include_directories(random_sampler PRIVATE
    ${CMAKE_SOURCE_DIR}/include/vara/Solver
)

target_link_libraries(random_sampler PRIVATE
  ${LLVM_LIBS}
)

include_directories(/opt/homebrew/Cellar/tinyxml2/11.0.0/include)
link_directories(/opt/homebrew/Cellar/tinyxml2/11.0.0/lib)
target_link_libraries(random_sampler PRIVATE tinyxml2)
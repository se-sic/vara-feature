set(SOLVER_LIB_SRC Z3Solver.cpp SolverFactory.cpp)

set(LLVM_LINK_COMPONENTS Core Support)

add_vara_library(VaRASolver ${SOLVER_LIB_SRC})

target_link_libraries(
  VaRASolver
  LINK_PUBLIC
  VaRAConfiguration
  VaRAFeature
  ${Z3_LIBRARIES}
)

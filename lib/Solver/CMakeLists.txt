set(SOLVER_LIB_SRC
  Solver.cpp
  SolverFactory.cpp
)

set(LLVM_LINK_COMPONENTS
)

add_vara_library(VaRASolver
  ${SOLVER_LIB_SRC}
)

target_link_libraries(VaRASolver
  LINK_PUBLIC
  VaRAConfiguration
)

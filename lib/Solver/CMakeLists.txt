set(SOLVER_LIB_SRC
  Solver.cpp
)

set(LLVM_LINK_COMPONENTS
)

add_vara_library(VaRASolver
  ${SOLVER_LIB_SRC}
)

# Solver sources
set(SOLVER_LIB_SRC
    Z3Solver.cpp
    SolverFactory.cpp
)

# Required LLVM components
set(LLVM_LINK_COMPONENTS Core Support)

# Define the actual solver library (STATIC)
add_library(VaRASolver STATIC ${SOLVER_LIB_SRC})

# Public include dirs (if Solver has headers in this folder)
target_include_directories(VaRASolver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(VaRASolver
    PUBLIC
        VaRAConfiguration
        VaRAFeature
        ${Z3_LIBRARIES}
)

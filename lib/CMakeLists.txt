add_subdirectory(Configuration)
add_subdirectory(Feature)
add_subdirectory(Sampling)

if(VARA_FEATURE_USE_Z3_SOLVER)
  add_subdirectory(Solver)
endif()

if (${VARA_FEATURE_EXECUTE_Z3_LARGE_MODEL_TESTS})
  add_vara_unittest(VaRASolverUnitTests VaRALargeSolverTests
    LargeCaseStudyTests.cpp
  )
else()
  set(LLVM_OPTIONAL_SOURCES
    LargeCaseStudyTests.cpp
  )
endif()

add_vara_unittest(VaRASolverUnitTests VaRASolverTests
  BasicSolverTests.cpp
  Z3Tests.cpp
  SolverFactory.cpp
  ConfigurationFactory.cpp
  RealWorldCaseStudyTests.cpp
)

target_link_libraries(VaRASolverTests
  PUBLIC
  ${Z3_LIBRARIES}
)

add_vara_unittest(VaRASolverUnitTests VaRASolverTests
  BasicSolverTests.cpp
  Z3Tests.cpp
  SolverFactory.cpp
  ConfigurationFactory.cpp
  RealWorldCaseStudyTests.cpp
)

target_link_libraries(VaRASolverTests
  PUBLIC
  Z3_opt
)
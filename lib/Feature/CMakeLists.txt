set(FEATURE_LIB_SRC
    Constraint.cpp
    Feature.cpp
    FeatureModel.cpp
    FeatureModelBuilder.cpp
    FeatureModelParser.cpp
    FeatureModelTransaction.cpp
    FeatureModelWriter.cpp
    OrderedFeatureVector.cpp
    UVL/AntrlUvl.cpp
)

set(LLVM_LINK_COMPONENTS Support Demangle Core)

add_vara_library(VaRAFeature ${FEATURE_LIB_SRC})

target_link_libraries(
  VaRAFeature LINK_PUBLIC ${LIBXML2_LIBRARIES} ${STD_FS_LIB}
)


set(ANTLR_RUNTIME_PATH /usr/local)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${ANTLR_RUNTIME_PATH})

find_library(ANTLR_RUNTIME_LIBRARY antlr4-runtime PATHS ${ANTLR_RUNTIME_PATH}/lib)
find_path(ANTLR_RUNTIME_INCLUDE_DIR antlr4-runtime.h PATHS ${ANTLR_RUNTIME_PATH}/include/antlr4-runtime)


if (ANTLR_RUNTIME_LIBRARY AND ANTLR_RUNTIME_INCLUDE_DIR)
    message(STATUS "Found ANTLR runtime at ${ANTLR_RUNTIME_PATH}")
    include_directories(${ANTLR_RUNTIME_INCLUDE_DIR})
else()
    message(FATAL_ERROR "ANTLR runtime not found. Set ANTLR_RUNTIME_PATH manually.")
endif()

include_directories(${ANTLR_RUNTIME_INCLUDE_DIR})

add_library(ANTLR4_RUNTIME STATIC IMPORTED)
set_target_properties(ANTLR4_RUNTIME PROPERTIES
        IMPORTED_LOCATION ${ANTLR_RUNTIME_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${ANTLR_RUNTIME_INCLUDE_DIR}
)

set(ANTLR_JAR /usr/local/lib/antlr-4.13.1-complete.jar)
set(ANTLR_COMMAND java -jar ${ANTLR_JAR})
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

# Define grammar file
set(GRAMMAR_FILE ${CMAKE_SOURCE_DIR}/lib/Feature/UVL/UVLcpp.g4)
set(LEXER_FILE ${CMAKE_SOURCE_DIR}/lib/Feature/UVL/UVLcppLexer.g4)



# Generate lexer/parser files
add_custom_command(
        OUTPUT ${GENERATED_DIR}/UVLcppLexer.cpp ${GENERATED_DIR}/UVLcppParser.cpp
        COMMAND ${ANTLR_COMMAND} -Dlanguage=Cpp -o ${GENERATED_DIR} ${GRAMMAR_FILE} ${LEXER_FILE}
        DEPENDS ${GRAMMAR_FILE}
        COMMENT "Generating ANTLR lexer/parser from ${GRAMMAR_FILE}"
)
message(STATUS "Generated directory is: ${GENERATED_DIR}")

include_directories(${GENERATED_DIR})


add_library(UVLParserLib STATIC ${GENERATED_DIR}/UVLcppLexer.cpp ${GENERATED_DIR}/UVLcppParser.cpp)
target_link_libraries(UVLParserLib PRIVATE antlr4-runtime)
target_include_directories(UVLParserLib PRIVATE ${GENERATED_DIR})

add_executable(AntlrTest UVL/AntrlUvl.cpp)
target_link_libraries(AntlrTest PRIVATE UVLParserLib antlr4-runtime)

# Add ANTLR runtime
#find_package(ANTLR REQUIRED)
#include_directories(${ANTLR_INCLUDE_DIRS})
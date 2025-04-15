#[[
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
]]
add_library(uvl_lib STATIC
        AntrlUvl.cpp
)

target_include_directories(uvl_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


find_file(ANTLR_JAR
        NAMES antlr-4.13.1-complete.jar
        PATHS
        /usr/local/lib
        /usr/share/java
        /usr/lib
        ~/Downloads
        DOC "Path to the ANTLR jar"
)


find_path(ANTLR4_RUNTIME_INCLUDE_DIR
        NAMES antlr4-runtime.h
        PATHS /usr/local/include/antlr4-runtime /usr/include
)
message(ANTLR_JAR = "${ANTLR4_RUNTIME_INCLUDE_DIR}")

include_directories(${ANTLR4_RUNTIME_INCLUDE_DIR})


add_library(ANTLR4_RUNTIME STATIC IMPORTED)
set_target_properties(ANTLR4_RUNTIME PROPERTIES
        IMPORTED_LOCATION ${ANTLR_RUNTIME_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${ANTLR_RUNTIME_INCLUDE_DIR}
)

#set(ANTLR_JAR /usr/local/lib/antlr-4.13.1-complete.jar)
set(ANTLR_COMMAND java -jar ${ANTLR_JAR})
set(LEXER_GENERATED_DIR ${CMAKE_BINARY_DIR}/lexer)
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

# Define grammar file
set(BASE_FILE ${CMAKE_SOURCE_DIR}/lib/Feature/UVL/UVLBase.g4)
set(GRAMMAR_FILE ${CMAKE_SOURCE_DIR}/lib/Feature/UVL/UVLcpp.g4)
set(LEXER_FILE ${CMAKE_SOURCE_DIR}/lib/Feature/UVL/UVLcppLexer.g4)


add_custom_command(
        OUTPUT ${GENERATED_DIR}/UVLcppLexer.h ${GENERATED_DIR}/UVLcppLexer.cpp
        COMMAND ${ANTLR_COMMAND} -Dlanguage=Cpp -o ${GENERATED_DIR} ${LEXER_FILE}
        DEPENDS ${LEXER_FILE}
)

# Generate lexer/parser files
add_custom_command(
        OUTPUT ${GENERATED_DIR}/UVLcppParser.cpp
        COMMAND ${ANTLR_COMMAND} -Dlanguage=Cpp -lib ${GENERATED_DIR} -o ${GENERATED_DIR} ${GRAMMAR_FILE}
        DEPENDS ${GRAMMAR_FILE}
        COMMENT "Generating ANTLR lexer/parser from ${GRAMMAR_FILE}"
)
message(STATUS "Generated directory is: ${GENERATED_DIR}")

include_directories(${LEXER_OUTPUT_DIR} ${GENERATED_DIR})


add_library(UVLParserLib STATIC ${GENERATED_DIR}/UVLcppLexer.cpp ${GENERATED_DIR}/UVLcppParser.cpp)
target_link_libraries(UVLParserLib PRIVATE antlr4-runtime)
target_include_directories(UVLParserLib PRIVATE ${GENERATED_DIR})

add_executable(AntlrTest AntrlUvl.cpp)
target_link_libraries(AntlrTest PRIVATE UVLParserLib antlr4-runtime)

# Add ANTLR runtime
#
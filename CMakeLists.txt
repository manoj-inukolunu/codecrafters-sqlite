cmake_minimum_required(VERSION 3.13)
project(sqlite-starter-cpp)

set(CMAKE_CXX_STANDARD 23)
option(BUILD_TESTS "Build unit tests" ON)

# --- ANTLR static runtime (imported) ---
set(ANTLR4_TAR "${CMAKE_SOURCE_DIR}/ant.tar.gz")
set(ANTLR4_EXTRACT_DIR "${CMAKE_SOURCE_DIR}")
if (NOT EXISTS "${ANTLR4_EXTRACT_DIR}/third_party/libantlr4-runtime.a")
    message(STATUS "Extracting ANTLR4 runtime from ${ANTLR4_TAR}")
    file(MAKE_DIRECTORY "${ANTLR4_EXTRACT_DIR}")
    file(ARCHIVE_EXTRACT INPUT "${ANTLR4_TAR}" DESTINATION "${ANTLR4_EXTRACT_DIR}")
endif ()

add_library(antlr4_runtime STATIC IMPORTED)
set_target_properties(antlr4_runtime PROPERTIES
        IMPORTED_LOCATION "${ANTLR4_EXTRACT_DIR}/third_party/libantlr4-runtime.a"
)

# --- Sources: define BEFORE using them ---
set(GENERATED
        src/gen_sqlite/SQLiteLexer.cpp
        src/gen_sqlite/SQLiteParser.cpp
        src/gen_sqlite/SQLiteParserBaseVisitor.cpp
        src/gen_sqlite/SQLiteParserVisitor.cpp
        src/CellParser.cpp
        src/catalog/CatalogBuilder.cpp
        src/catalog/CatalogBuilder.h
        src/common/Sqlite.h
        src/Node.cpp
        src/Node.h
        src/parser/parser.h
        src/parser/CreateStatement.cpp
        src/parser/SqlStatement.cpp
        src/parser/SqlStatement.h
        src/parser/CreateTableStatement.h
        src/parser/ColumnDefinition.cpp
        src/parser/ColumnDefinition.h
        src/parser/Constraint.cpp
        src/parser/Constraint.h
        src/parser/SelectStatement.cpp
        src/parser/SelectStatement.h
        src/parser/BaseTableRef.cpp
        src/parser/TableRef.cpp
        src/parser/TableRef.h
        src/parser/BaseTableRef.cpp
        src/parser/BaseTableRef.h
        src/parser/ParsedExpression.cpp
        src/parser/ParsedExpression.h
)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
        src/*.cpp
        src/*.hpp
)

# --- ANTLR grammar lib ---
include_directories(third_party/antlr4-runtime)
include_directories(src/gen_sqlite/)
add_library(sqlite_grammar
        src/gen_sqlite/SQLiteLexer.cpp
        src/gen_sqlite/SQLiteParser.cpp
        src/gen_sqlite/SQLiteParserBaseVisitor.cpp
        src/gen_sqlite/SQLiteParserVisitor.cpp
)
target_include_directories(sqlite_grammar PUBLIC gen_sqlite)
target_link_libraries(sqlite_grammar PUBLIC antlr4_runtime)

# --- Main executable ---
add_executable(exe ${SOURCE_FILES} ${GENERATED})
target_include_directories(exe PRIVATE gen_sqlite)
target_link_libraries(exe PRIVATE sqlite_grammar antlr4_runtime)

# --- Tests (optional) ---
if (BUILD_TESTS)
    enable_testing()

    # Use Catch2 amalgamated: compile its .cpp OR provide CATCH_CONFIG_MAIN in one file
    add_executable(unit_tests
            tests/catch_amalgamated.cpp    # <- preferred (don’t also define CATCH_CONFIG_MAIN)
            tests/test_parse.cpp
            # you can add more test files here
    )

    target_include_directories(unit_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/catch2   # contains catch_amalgamated.hpp/.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src                  # your headers
    )

    # Link your code if tests need it (instead of recompiling all src into tests)
    target_link_libraries(unit_tests PRIVATE sqlite_grammar antlr4_runtime)

    # Register with CTest (so CLion’s CTest runner sees it)
    add_test(NAME all-tests COMMAND unit_tests)
endif ()

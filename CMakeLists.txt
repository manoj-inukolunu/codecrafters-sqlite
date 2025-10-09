cmake_minimum_required(VERSION 3.13)

project(sqlite-starter-cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard


#[[add_subdirectory(third_party/antlr4_runtime ${CMAKE_BINARY_DIR}/antlr4_runtime)
set(ANTLR4_RUNTIME_TARGET antlr4`1_static)]]

#find_package(antlr4  CONFIG REQUIRED)

set(ANTLR4_TAR "${CMAKE_SOURCE_DIR}/ant.tar.gz")
set(ANTLR4_EXTRACT_DIR "${CMAKE_SOURCE_DIR}")

if(NOT EXISTS "${ANTLR4_EXTRACT_DIR}/third_party/libantlr4-runtime.a")
    message(STATUS "Extracting ANTLR4 runtime from ${ANTLR4_TAR}")
    file(MAKE_DIRECTORY "${ANTLR4_EXTRACT_DIR}")
    file(ARCHIVE_EXTRACT INPUT "${ANTLR4_TAR}" DESTINATION "${ANTLR4_EXTRACT_DIR}")
endif()

add_library(antlr4_runtime STATIC IMPORTED)
set_target_properties(antlr4_runtime PROPERTIES
        IMPORTED_LOCATION             "${ANTLR4_EXTRACT_DIR}/third_party/libantlr4-runtime.a"
)




set(GENERATED
        src/gen_sqlite/SQLiteLexer.cpp
        src/gen_sqlite/SQLiteParser.cpp
        src/gen_sqlite/SQLiteParserBaseVisitor.cpp
        src/gen_sqlite/SQLiteParserVisitor.cpp
        src/CellParser.cpp
        src/catalog/CatalogBuilder.cpp
        src/catalog/CatalogBuilder.h
        src/Sqlite.h
        src/Node.cpp
        src/Node.h
        src/parser/parser.h
        src/parser/CreateStatement.cpp
        src/parser/SqlStatement.cpp
        src/parser/SqlStatement.h
        src/parser/CreateStatement.h
)


file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

include_directories(third_party/antlr4-runtime)
include_directories(src/gen_sqlite/)
add_library(sqlite_grammar
        src/gen_sqlite/SQLiteLexer.cpp
        src/gen_sqlite/SQLiteParser.cpp
        src/gen_sqlite/SQLiteParserBaseVisitor.cpp
        src/gen_sqlite/SQLiteParserVisitor.cpp
)
#set(ANTLR_LIB "${CMAKE_SOURCE_DIR}/third_party/libantlr4-runtime.a")

target_include_directories(sqlite_grammar PUBLIC gen_sqlite)
target_link_libraries(sqlite_grammar PUBLIC antlr4_runtime)


add_executable(exe ${SOURCE_FILES} ${GENERATED}
        src/SqliteSchemaPageReader.cpp
        src/SqliteSchemaPageReader.h
        src/utils.h
        src/SqlitePageReader.cpp
        src/SqlitePageReader.h
        src/SqliteUtils.cpp
        src/SqliteUtils.h
        src/SQLitePrinter.cpp
)


target_include_directories(exe PRIVATE gen_sqlite)
target_link_libraries(exe PRIVATE sqlite_grammar antlr4_runtime)




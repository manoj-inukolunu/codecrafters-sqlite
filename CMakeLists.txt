cmake_minimum_required(VERSION 3.13)

project(sqlite-starter-cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard


#[[add_subdirectory(third_party/antlr4_runtime ${CMAKE_BINARY_DIR}/antlr4_runtime)
set(ANTLR4_RUNTIME_TARGET antlr4_static)]]

find_package(antlr4  CONFIG REQUIRED)


set(GENERATED
        src/gen_sqlite/SQLiteLexer.cpp
        src/gen_sqlite/SQLiteParser.cpp
        src/gen_sqlite/SQLiteParserBaseVisitor.cpp
        src/gen_sqlite/SQLiteParserVisitor.cpp
        src/HeaderParser.cpp
)


file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

include_directories(thrid_party/include/antlr4-runtime)
include_directories(src/gen_sqlite/)
add_library(sqlite_grammar
        src/gen_sqlite/SQLiteLexer.cpp
        src/gen_sqlite/SQLiteParser.cpp
        src/gen_sqlite/SQLiteParserBaseVisitor.cpp
        src/gen_sqlite/SQLiteParserVisitor.cpp
)
target_include_directories(sqlite_grammar PUBLIC gen_sqlite)
target_link_libraries(sqlite_grammar PUBLIC ${ANTLR4_RUNTIME_TARGET})


add_executable(exe ${SOURCE_FILES} ${GENERATED}
        src/SqliteSchemaPageReader.cpp
        src/SqliteSchemaPageReader.h
        src/utils.h
        src/SqlitePageReader.cpp
        src/SqlitePageReader.h
        src/SqliteUtils.cpp
        src/SqliteUtils.h
        src/sqlite_main.cpp
        src/SQLitePrinter.cpp
        src/SQLitePrinter.cpp
        src/SQLitePrinter.hpp
        src/sqlite_main.cpp
)

target_include_directories(exe PRIVATE gen_sqlite)
target_link_libraries(exe PRIVATE sqlite_grammar ${ANTLR4_RUNTIME_TARGET})
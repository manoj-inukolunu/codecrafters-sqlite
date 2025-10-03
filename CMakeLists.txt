cmake_minimum_required(VERSION 3.13)

project(sqlite-starter-cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(exe ${SOURCE_FILES}
        src/SqliteBTreePageReader.cpp
        src/SqliteSchemaPageReader.h
        src/SqliteTable.cpp
        src/SqliteTable.h
        src/SqliteCellReader.cpp
        src/SqliteCellReader.h
        src/types.h
        src/SqliteRecord.cpp
        src/SqliteRecord.h)
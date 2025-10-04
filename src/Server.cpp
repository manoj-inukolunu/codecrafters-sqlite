#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <unordered_map>

#include "SqliteSchemaPageReader.h"

static const int DATABASE_HEADER_SIZE_BYTES = 100;
static const int CELL_OFFSET = 3;
static const int PAGE_SIZE_OFFSET = 16;


enum Command {
    DBINFO, TABLES, INVALID
};


uint16_t swap(uint16_t x) {
    return (x >> 8) | (x << 8);
}

bool little_endian() {
    int x = 1;
    return (*reinterpret_cast<char *>(&x) == 1);
}

int read2Bytes(std::ifstream &file, int offset) {
    file.seekg(offset, std::ios::beg);
    size_t n = 2;
    uint16_t value = 0;
    file.read(reinterpret_cast<char *>(&value), n);
    if (little_endian()) {
        value = swap(value);
    }
    file.close();
    return value;
}

int file_reader(const std::string &file_location) {
    //    std::cout << "Reading dbFile " << file_location << std::endl;
    std::ifstream file(file_location, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error could not open dbFile");
    }
    int val = read2Bytes(file, PAGE_SIZE_OFFSET);
    return val;
}


int numCellsInFirstPage(const std::string &file_location) {
    //read dbFile in binary mode
    std::ifstream file(file_location, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("File is not open , I don't know why");
    }
    int offset = DATABASE_HEADER_SIZE_BYTES + CELL_OFFSET;
    int val = read2Bytes(file, offset);
    std::cout << "number of tables: " << val << std::endl;

    return val;
}


int _main(int args, char **argv) {
    std::cout << argv[1] << std::endl;
    std::string location = argv[1] + std::string("/sample.db");
    int pageSize = file_reader(location);
    std::ifstream dbFile(location, std::ios::binary);
    SqliteSchemaPageReader reader(0, pageSize, dbFile);

    std::cout << reader.cellContentAreaStart << std::endl;

}

int main(int argc, char *argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here" << std::endl;

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string database_file_path = argv[1];
    std::string command = argv[2];

    std::unordered_map<std::string, Command> commandMap = {
            {".dbinfo", DBINFO},
            {".tables", TABLES}
    };

    Command c = commandMap.count(command) ? commandMap[command] : INVALID;

    switch (c) {
        case DBINFO: {
            std::ifstream database_file(database_file_path, std::ios::binary);
            if (!database_file) {
                std::cerr << "Failed to open the database dbFile" << std::endl;
                return 1;
            }

            int val = file_reader(database_file_path);
            std::cout << "database page size: " << val << std::endl;
            numCellsInFirstPage(database_file_path);
            break;
        }
        case TABLES: {
            int pageSize = file_reader(database_file_path);
            std::ifstream dbFile(database_file_path, std::ios::binary);
            SqliteSchemaPageReader reader(1, pageSize, dbFile);
            reader.printTableNames();
            break;
        }
        default:
            throw std::runtime_error("Fail");
    }
    return 0;
}

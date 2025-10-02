//
// Created by Manoj Inukolunu on 02/10/25.
//

#ifndef SQLITE_STARTER_CPP_SQLITERECORD_H
#define SQLITE_STARTER_CPP_SQLITERECORD_H


#include "types.h"
//Also called payload from https://www.sqlite.org/fileformat.html#record_format
class SqliteRecord {
    struct Header {
        varint headerSize;
        varint serialTypeCode;
    };
};


#endif //SQLITE_STARTER_CPP_SQLITERECORD_H

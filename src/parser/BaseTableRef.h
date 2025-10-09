//
// Created by Manoj on 09-10-2025.
//

#ifndef BASETABLEREF_H
#define BASETABLEREF_H
#include "TableRef.h"


class BaseTableRef : TableRef {
public:
    explicit BaseTableRef()
            : TableRef(TableReferenceType::BASE_TABLE) {
    }

    std::string tableName;
    std::string schemaName;
};


#endif //BASETABLEREF_H

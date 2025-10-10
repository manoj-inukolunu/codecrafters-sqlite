//
// Created by Manoj on 09-10-2025.
//

#ifndef TABLEREF_H
#define TABLEREF_H
#include "parser.h"


class TableRef {
public:
    TableRef(TableReferenceType type): type(type) {
    }

    TableReferenceType type;
};


#endif //TABLEREF_H

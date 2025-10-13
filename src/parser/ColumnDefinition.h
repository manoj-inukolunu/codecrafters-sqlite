//
// Created by Manoj on 09-10-2025.
//

#ifndef COLUMNDEFINITION_H
#define COLUMNDEFINITION_H
#include <string>


#include "Constraint.h"
#include "../common/Sqlite.h"

class ColumnDefinition {
public :
    ColumnDefinition(std::string name, DataType type): name(name), type(type) {
    }

    std::string name;
    //TODO: In the parser and the grammar make this not optional later.
    DataType type;
    std::vector<std::shared_ptr<Constraint>> constraints;
    // No Default value support now
};


#endif //COLUMNDEFINITION_H

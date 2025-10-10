//
// Created by Manoj on 09-10-2025.
//

#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "../common/Sqlite.h"


enum class ConstraintType {
    TABLE_CONSTRAINT,
    COLUMN_CONSTRAINT
};

class Constraint {
public:
    Constraint(ConstraintType type): constraintType(type) {
    }

    ConstraintType constraintType;
    std::string name;



};


#endif //CONSTRAINT_H

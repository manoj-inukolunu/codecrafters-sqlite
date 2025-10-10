//
// Created by Manoj on 09-10-2025.
//

#ifndef PARSEDEXPRESSION_H
#define PARSEDEXPRESSION_H

#include "../common/Sqlite.h"
#include <memory>

class ParsedExpression {
public:
    std::shared_ptr<ParsedExpression> left;
    std::string op; // Operator like "+", "-", etc.
    std::shared_ptr<ParsedExpression> right;

    ParsedExpression() = default;

    std::string value; // For literals or column names

    ParsedExpression(std::string oper) : op(std::move(oper)) {
    }
};


#endif //PARSEDEXPRESSION_H

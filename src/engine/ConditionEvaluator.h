#pragma once

#include "Ast.h"
#include "Record.h"
#include "TableSchema.h"

class ConditionEvaluator {
public:
    bool evaluate(const TableSchema& schema,
                  const Record& record,
                  const Condition& condition) const;
};

#pragma once

#include <string>
#include <vector>

#include "Ast.h"
#include "Record.h"
#include "TableSchema.h"

class JsonFormatter {
public:
    std::string formatRows(const TableSchema& schema,
                           const std::vector<Record>& records,
                           bool selectAll,
                           const std::vector<SelectItem>& items) const;
};

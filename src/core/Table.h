#pragma once

#include <optional>
#include <vector>

#include "Field.h"
#include "Record.h"

class Table {
public:
    virtual ~Table() = default;

    virtual void insert(const Record& record) = 0;
    virtual std::vector<Record> scan() = 0;
    virtual std::optional<Record> findByIndex(int key) = 0;
};

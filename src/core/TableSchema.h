#pragma once

#include <cstddef>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "Field.h"

struct Column {
    std::string name;
    FieldType type{FieldType::INT};
    bool indexed{false};

    Column() = default;
    Column(std::string columnName, FieldType columnType, bool isIndexed = false)
        : name(std::move(columnName)), type(columnType), indexed(isIndexed) {}

    static Column Integer(std::string name, bool indexed = false) {
        return Column(std::move(name), FieldType::INT, indexed);
    }

    static Column Text(std::string name, bool indexed = false) {
        return Column(std::move(name), FieldType::STRING, indexed);
    }
};

struct TableSchema {
    std::vector<Column> columns;

    TableSchema() = default;
    explicit TableSchema(std::vector<Column> values)
        : columns(std::move(values)) {}

    TableSchema(std::initializer_list<Column> values)
        : columns(values) {}

    void addColumn(const Column& column) {
        columns.push_back(column);
    }

    std::size_t size() const {
        return columns.size();
    }

    bool empty() const {
        return columns.empty();
    }

    const Column* findColumn(const std::string& name) const {
        for (const auto& column : columns) {
            if (column.name == name) {
                return &column;
            }
        }
        return nullptr;
    }
};

#include "JsonFormatter.h"

#include <map>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace {

std::map<std::string, std::size_t> buildColumnIndex(const TableSchema& schema) {
    std::map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < schema.columns.size(); ++i) {
        columns[schema.columns[i].name] = i;
    }
    return columns;
}

std::vector<SelectItem> expandSelectAll(const TableSchema& schema) {
    std::vector<SelectItem> items;
    items.reserve(schema.columns.size());
    for (const Column& column : schema.columns) {
        items.push_back(SelectItem{column.name, std::nullopt});
    }
    return items;
}

std::string outputName(const SelectItem& item) {
    return item.alias.value_or(item.columnName);
}

}

std::string JsonFormatter::formatRows(const TableSchema& schema,
                                      const std::vector<Record>& records,
                                      bool selectAll,
                                      const std::vector<SelectItem>& items) const {
    const std::map<std::string, std::size_t> columnIndex = buildColumnIndex(schema);
    const std::vector<SelectItem> selectedItems = selectAll ? expandSelectAll(schema) : items;

    for (const SelectItem& item : selectedItems) {
        if (columnIndex.find(item.columnName) == columnIndex.end()) {
            throw std::runtime_error("Column '" + item.columnName + "' does not exist");
        }
    }

    nlohmann::ordered_json result = nlohmann::ordered_json::array();
    for (const Record& record : records) {
        nlohmann::ordered_json row = nlohmann::ordered_json::object();
        for (const SelectItem& item : selectedItems) {
            const Field& field = record.fields.at(columnIndex.at(item.columnName));
            if (field.isInt()) {
                row[outputName(item)] = field.intValue;
            } else {
                row[outputName(item)] = field.stringValue;
            }
        }
        result.push_back(std::move(row));
    }

    return result.dump();
}

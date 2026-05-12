#include "Table.h"

#include <stdexcept>
#include <memory>
#include <cstddef>
#include <utility>

#include "FileManager.h"

namespace {

std::optional<std::size_t> findIndexedColumnPosition(const TableSchema& schema) {
    for (std::size_t i = 0; i < schema.columns.size(); ++i) {
        if (schema.columns[i].indexed) {
            return i;
        }
    }

    return std::nullopt;
}

std::optional<int> extractIndexedKey(const TableSchema& schema, const Record& record) {
    const std::optional<std::size_t> indexedColumnPosition = findIndexedColumnPosition(schema);
    if (!indexedColumnPosition.has_value()) {
        return std::nullopt;
    }

    if (*indexedColumnPosition >= record.fields.size()) {
        throw std::runtime_error("Record does not contain indexed field");
    }

    const Field& field = record.fields[*indexedColumnPosition];
    if (!field.isInt()) {
        throw std::runtime_error("Indexed field must have INT type");
    }

    return field.intValue;
}

bool schemasEqual(const TableSchema& lhs, const TableSchema& rhs) {
    if (lhs.columns.size() != rhs.columns.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.columns.size(); ++i) {
        const Column& leftColumn = lhs.columns[i];
        const Column& rightColumn = rhs.columns[i];
        if (leftColumn.name != rightColumn.name ||
            leftColumn.type != rightColumn.type ||
            leftColumn.indexed != rightColumn.indexed) {
            return false;
        }
    }

    return true;
}

}

Table::Table(std::filesystem::path filePath, const TableSchema& schema) {
    initialize(std::move(filePath), schema);
}

Table::~Table() {
    delete fileManager_;
}

void Table::initialize(std::filesystem::path filePath, const TableSchema& schema) {
    const bool fileExists = std::filesystem::exists(filePath);
    const bool fileHasData = fileExists && std::filesystem::is_regular_file(filePath) &&
        std::filesystem::file_size(filePath) > 0;
    std::unique_ptr<FileManager> newManager = std::make_unique<FileManager>(std::move(filePath));

    if (fileHasData) {
        const TableSchema storedSchema = newManager->readSchema();
        if (!schemasEqual(storedSchema, schema)) {
            throw std::runtime_error("Existing table schema does not match requested schema");
        }
        schema_ = storedSchema;
    } else {
        schema_ = schema;
        newManager->writeSchema(schema_);
    }

    delete fileManager_;
    fileManager_ = newManager.release();
}

bool Table::isConfigured() const {
    return static_cast<bool>(fileManager_);
}

void Table::insert(const Record& record) {
    if (!isConfigured()) {
        throw std::runtime_error("Table storage is not configured");
    }

    const std::streampos offset = fileManager_->writeRecord(record);

    if (index != nullptr) {
        const std::optional<int> key = extractIndexedKey(schema_, record);
        if (key.has_value()) {
            index->insert(*key, static_cast<int>(offset));
        }
    }
}

std::vector<Record> Table::scan() {
    if (!isConfigured()) {
        throw std::runtime_error("Table storage is not configured");
    }

    return fileManager_->readAllRecords();
}

std::optional<Record> Table::findByIndex(int key) {
    if (!isConfigured()) {
        throw std::runtime_error("Table storage is not configured");
    }

    if (index != nullptr) {
        const std::optional<int> offset = index->find(key);
        if (!offset.has_value()) {
            return std::nullopt;
        }

        return fileManager_->readRecord(static_cast<std::streampos>(*offset));
    }

    const std::optional<std::size_t> indexedColumnPosition = findIndexedColumnPosition(schema_);
    if (!indexedColumnPosition.has_value()) {
        return std::nullopt;
    }

    for (const Record& record : scan()) {
        if (*indexedColumnPosition >= record.fields.size()) {
            continue;
        }

        const Field& field = record.fields[*indexedColumnPosition];
        if (field.isInt() && field.intValue == key) {
            return record;
        }
    }

    return std::nullopt;
}

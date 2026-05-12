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

}

Table::Table(std::filesystem::path filePath, const TableSchema& schema) {
    initialize(std::move(filePath), schema);
}

Table::~Table() {
    delete fileManager_;
}

void Table::initialize(std::filesystem::path filePath, const TableSchema& schema) {
    schema_ = schema;
    std::unique_ptr<FileManager> newManager = std::make_unique<FileManager>(std::move(filePath));
    newManager->writeSchema(schema_);
    delete fileManager_;
    fileManager_ = newManager.release();
    recordOffsets_.clear();
}

bool Table::isConfigured() const {
    return static_cast<bool>(fileManager_);
}

void Table::insert(const Record& record) {
    if (!isConfigured()) {
        throw std::runtime_error("Table storage is not configured");
    }

    const std::streampos offset = fileManager_->writeRecord(record);
    recordOffsets_.push_back(offset);

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

    std::vector<Record> records;
    records.reserve(recordOffsets_.size());
    for (const auto offset : recordOffsets_) {
        records.push_back(fileManager_->readRecord(offset));
    }
    return records;
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

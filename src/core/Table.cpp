#include "Table.h"

#include <stdexcept>
#include <memory>
#include <utility>

#include "FileManager.h"

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

    if (key < 0 || static_cast<std::size_t>(key) >= recordOffsets_.size()) {
        return std::nullopt;
    }

    return fileManager_->readRecord(recordOffsets_[static_cast<std::size_t>(key)]);
}

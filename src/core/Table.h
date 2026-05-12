#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "Field.h"
#include "Index.h"
#include "Record.h"
#include "TableSchema.h"

class FileManager;

class Table {
public:
    Index* index = nullptr;

    Table() = default;
    explicit Table(std::filesystem::path filePath, const TableSchema& schema);
    virtual ~Table();

    Table(const Table&) = delete;
    Table& operator=(const Table&) = delete;
    Table(Table&&) = delete;
    Table& operator=(Table&&) = delete;

    void initialize(std::filesystem::path filePath, const TableSchema& schema);

    virtual void insert(const Record& record);
    void insertFromParser(const Record& record);
    virtual std::vector<Record> scan();
    virtual std::optional<Record> findByIndex(int key);
    std::optional<Record> selectByKey(int key);

protected:
    bool isConfigured() const;

private:
    FileManager* fileManager_{nullptr};
    TableSchema schema_;
};

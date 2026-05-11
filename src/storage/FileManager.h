#pragma once

#include <filesystem>
#include <optional>
#include <fstream>

#include "Record.h"
#include "TableSchema.h"

class FileManager {
public:
    explicit FileManager(std::filesystem::path filePath);

    void writeSchema(const TableSchema& schema);
    TableSchema readSchema();

    std::streampos writeRecord(const Record& record);
    Record readRecord(std::streampos offset);

private:
    std::filesystem::path filePath_;
    std::optional<TableSchema> schema_;
    std::streampos dataStart_{0};

    void ensureSchemaLoaded();
};

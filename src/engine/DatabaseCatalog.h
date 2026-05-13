#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "Ast.h"
#include "TableSchema.h"

struct ResolvedTableName {
    std::string databaseName;
    std::string tableName;
    std::filesystem::path tablePath;
};

class DatabaseCatalog {
public:
    explicit DatabaseCatalog(std::filesystem::path rootPath = "data");

    const std::filesystem::path& rootPath() const;

    bool databaseExists(const std::string& databaseName) const;
    void createDatabase(const std::string& databaseName);
    void dropDatabase(const std::string& databaseName);
    void useDatabase(const std::string& databaseName);

    const std::optional<std::string>& activeDatabase() const;
    ResolvedTableName resolveTableName(const TableName& tableName) const;

    bool tableExists(const TableName& tableName) const;
    void createTable(const TableName& tableName, const TableSchema& schema);
    void dropTable(const TableName& tableName);
    TableSchema readTableSchema(const TableName& tableName) const;

private:
    std::filesystem::path rootPath_;
    std::optional<std::string> activeDatabase_;

    std::filesystem::path databasePath(const std::string& databaseName) const;
    std::filesystem::path tablePath(const std::string& databaseName,
                                    const std::string& tableName) const;
};

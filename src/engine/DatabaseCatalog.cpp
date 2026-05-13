#include "DatabaseCatalog.h"

#include <utility>

#include "EngineError.h"
#include "FileManager.h"
#include "Table.h"

DatabaseCatalog::DatabaseCatalog(std::filesystem::path rootPath)
    : rootPath_(std::move(rootPath)) {}

const std::filesystem::path& DatabaseCatalog::rootPath() const {
    return rootPath_;
}

bool DatabaseCatalog::databaseExists(const std::string& databaseName) const {
    const std::filesystem::path path = databasePath(databaseName);
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

void DatabaseCatalog::createDatabase(const std::string& databaseName) {
    if (databaseExists(databaseName)) {
        throw EngineError("Database '" + databaseName + "' already exists");
    }

    const std::filesystem::path path = databasePath(databaseName);
    if (std::filesystem::exists(path)) {
        throw EngineError("Path for database '" + databaseName + "' already exists");
    }

    if (!std::filesystem::create_directories(path)) {
        throw EngineError("Failed to create database '" + databaseName + "'");
    }
}

void DatabaseCatalog::dropDatabase(const std::string& databaseName) {
    if (!databaseExists(databaseName)) {
        throw EngineError("Database '" + databaseName + "' does not exist");
    }

    std::filesystem::remove_all(databasePath(databaseName));
    if (activeDatabase_.has_value() && *activeDatabase_ == databaseName) {
        activeDatabase_.reset();
    }
}

void DatabaseCatalog::useDatabase(const std::string& databaseName) {
    if (!databaseExists(databaseName)) {
        throw EngineError("Database '" + databaseName + "' does not exist");
    }

    activeDatabase_ = databaseName;
}

const std::optional<std::string>& DatabaseCatalog::activeDatabase() const {
    return activeDatabase_;
}

ResolvedTableName DatabaseCatalog::resolveTableName(const TableName& tableName) const {
    const std::string databaseName = tableName.databaseName.has_value()
        ? *tableName.databaseName
        : activeDatabase_.value_or("");

    if (databaseName.empty()) {
        throw EngineError("No active database selected");
    }

    if (!databaseExists(databaseName)) {
        throw EngineError("Database '" + databaseName + "' does not exist");
    }

    return ResolvedTableName{
        databaseName,
        tableName.tableName,
        tablePath(databaseName, tableName.tableName)
    };
}

bool DatabaseCatalog::tableExists(const TableName& tableName) const {
    const ResolvedTableName resolved = resolveTableName(tableName);
    return std::filesystem::exists(resolved.tablePath) &&
        std::filesystem::is_regular_file(resolved.tablePath);
}

void DatabaseCatalog::createTable(const TableName& tableName, const TableSchema& schema) {
    const ResolvedTableName resolved = resolveTableName(tableName);
    if (tableExists(tableName)) {
        throw EngineError("Table '" + resolved.databaseName + "." +
                          resolved.tableName + "' already exists");
    }

    if (std::filesystem::exists(resolved.tablePath)) {
        throw EngineError("Path for table '" + resolved.databaseName + "." +
                          resolved.tableName + "' already exists");
    }

    Table table(resolved.tablePath, schema);
}

void DatabaseCatalog::dropTable(const TableName& tableName) {
    const ResolvedTableName resolved = resolveTableName(tableName);
    if (!tableExists(tableName)) {
        throw EngineError("Table '" + resolved.databaseName + "." +
                          resolved.tableName + "' does not exist");
    }

    std::filesystem::remove(resolved.tablePath);
}

TableSchema DatabaseCatalog::readTableSchema(const TableName& tableName) const {
    const ResolvedTableName resolved = resolveTableName(tableName);
    if (!tableExists(tableName)) {
        throw EngineError("Table '" + resolved.databaseName + "." +
                          resolved.tableName + "' does not exist");
    }

    FileManager reader(resolved.tablePath);
    return reader.readSchema();
}

std::filesystem::path DatabaseCatalog::databasePath(const std::string& databaseName) const {
    return rootPath_ / databaseName;
}

std::filesystem::path DatabaseCatalog::tablePath(const std::string& databaseName,
                                                 const std::string& tableName) const {
    return databasePath(databaseName) / (tableName + ".tbl");
}

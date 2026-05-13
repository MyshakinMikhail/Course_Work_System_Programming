#pragma once

#include "Ast.h"
#include "DatabaseCatalog.h"
#include "ExecutionResult.h"

class Engine {
public:
    Engine();
    explicit Engine(std::filesystem::path dataRoot);

    ExecutionResult execute(const ParsedCommand& command);

private:
    DatabaseCatalog catalog_;

    ExecutionResult executeCreateDatabase(const CreateDatabaseCommand& command);
    ExecutionResult executeDropDatabase(const DropDatabaseCommand& command);
    ExecutionResult executeUseDatabase(const UseDatabaseCommand& command);
    ExecutionResult executeCreateTable(const CreateTableCommand& command);
    ExecutionResult executeDropTable(const DropTableCommand& command);
    ExecutionResult executeInsert(const InsertCommand& command);
    ExecutionResult executeSelect(const SelectCommand& command);
    ExecutionResult executeUpdate(const UpdateCommand& command);
    ExecutionResult executeDelete(const DeleteCommand& command);
};

#include <cassert>
#include <filesystem>
#include <string>

#include "DatabaseCatalog.h"
#include "EngineError.h"

namespace {

template <typename Fn>
void expectEngineError(Fn&& fn) {
    bool thrown = false;
    try {
        fn();
    } catch (const EngineError&) {
        thrown = true;
    }
    assert(thrown);
}

std::filesystem::path makeTempRoot(const std::string& name) {
    const std::filesystem::path root = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(root);
    return root;
}

}

static void testCreateDropAndUseDatabase() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_catalog_test");

    DatabaseCatalog catalog(root);
    assert(!catalog.databaseExists("app"));

    catalog.createDatabase("app");
    assert(catalog.databaseExists("app"));
    assert(std::filesystem::is_directory(root / "app"));

    catalog.useDatabase("app");
    assert(catalog.activeDatabase().has_value());
    assert(*catalog.activeDatabase() == "app");

    catalog.dropDatabase("app");
    assert(!catalog.databaseExists("app"));
    assert(!catalog.activeDatabase().has_value());

    std::filesystem::remove_all(root);
}

static void testRejectsInvalidDatabaseOperations() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_catalog_errors_test");

    DatabaseCatalog catalog(root);

    expectEngineError([&]() {
        catalog.useDatabase("missing");
    });

    expectEngineError([&]() {
        catalog.dropDatabase("missing");
    });

    catalog.createDatabase("app");
    expectEngineError([&]() {
        catalog.createDatabase("app");
    });

    std::filesystem::remove_all(root);
}

static void testResolveTableName() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_catalog_resolve_test");

    DatabaseCatalog catalog(root);
    catalog.createDatabase("app");
    catalog.createDatabase("logs");
    catalog.useDatabase("app");

    const ResolvedTableName activeTable = catalog.resolveTableName(TableName{std::nullopt, "users"});
    assert(activeTable.databaseName == "app");
    assert(activeTable.tableName == "users");
    assert(activeTable.tablePath == root / "app" / "users.tbl");

    const ResolvedTableName qualifiedTable = catalog.resolveTableName(TableName{"logs", "events"});
    assert(qualifiedTable.databaseName == "logs");
    assert(qualifiedTable.tableName == "events");
    assert(qualifiedTable.tablePath == root / "logs" / "events.tbl");

    std::filesystem::remove_all(root);
}

static void testResolveRequiresActiveOrExistingDatabase() {
    const std::filesystem::path root = makeTempRoot("course_work_engine_catalog_resolve_errors_test");

    DatabaseCatalog catalog(root);
    expectEngineError([&]() {
        (void)catalog.resolveTableName(TableName{std::nullopt, "users"});
    });

    expectEngineError([&]() {
        (void)catalog.resolveTableName(TableName{"missing", "users"});
    });

    std::filesystem::remove_all(root);
}

int main() {
    testCreateDropAndUseDatabase();
    testRejectsInvalidDatabaseOperations();
    testResolveTableName();
    testResolveRequiresActiveOrExistingDatabase();
    return 0;
}

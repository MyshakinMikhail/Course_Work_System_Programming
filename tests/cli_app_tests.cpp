#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "CliApp.h"

namespace {

std::filesystem::path makeTempRoot(const std::string& name) {
    const std::filesystem::path root = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(root);
    return root;
}

}

static void testBatchRunsCommandsAndPrintsResults() {
    const std::filesystem::path root = makeTempRoot("course_work_cli_batch_data_test");
    const std::filesystem::path scriptPath =
        std::filesystem::temp_directory_path() / "course_work_cli_batch_script.sql";

    {
        std::ofstream script(scriptPath);
        script << "CREATE DATABASE app;\n"
               << "USE app;\n"
               << "CREATE TABLE users (id int, name string);\n"
               << "INSERT INTO users (id, name) VALUE (1, \"Ann\");\n"
               << "SELECT * FROM users;\n";
    }

    CliApp app(root);
    std::ostringstream output;
    std::ostringstream error;

    const int exitCode = app.runBatch(scriptPath, output, error);
    assert(exitCode == 0);
    assert(error.str().empty());
    assert(output.str().find("Database 'app' created") != std::string::npos);
    assert(output.str().find("[{\"id\":1,\"name\":\"Ann\"}]") != std::string::npos);

    std::filesystem::remove(scriptPath);
    std::filesystem::remove_all(root);
}

static void testBatchContinuesAfterErrors() {
    const std::filesystem::path root = makeTempRoot("course_work_cli_batch_errors_data_test");
    const std::filesystem::path scriptPath =
        std::filesystem::temp_directory_path() / "course_work_cli_batch_errors_script.sql";

    {
        std::ofstream script(scriptPath);
        script << "USE missing;\n"
               << "CREATE DATABASE app;\n"
               << "USE app;\n";
    }

    CliApp app(root);
    std::ostringstream output;
    std::ostringstream error;

    const int exitCode = app.runBatch(scriptPath, output, error);
    assert(exitCode == 0);
    assert(error.str().find("Database 'missing' does not exist") != std::string::npos);
    assert(output.str().find("Database 'app' created") != std::string::npos);
    assert(output.str().find("Using database 'app'") != std::string::npos);

    std::filesystem::remove(scriptPath);
    std::filesystem::remove_all(root);
}

static void testBatchReportsMissingFile() {
    const std::filesystem::path root = makeTempRoot("course_work_cli_missing_file_data_test");
    const std::filesystem::path scriptPath =
        std::filesystem::temp_directory_path() / "course_work_cli_missing_file.sql";
    std::filesystem::remove(scriptPath);

    CliApp app(root);
    std::ostringstream output;
    std::ostringstream error;

    const int exitCode = app.runBatch(scriptPath, output, error);
    assert(exitCode == 1);
    assert(output.str().empty());
    assert(error.str().find("failed to open script file") != std::string::npos);

    std::filesystem::remove_all(root);
}

static void testInteractiveUsesPersistentEngine() {
    const std::filesystem::path root = makeTempRoot("course_work_cli_interactive_data_test");

    std::istringstream input(
        "CREATE DATABASE app;\n"
        "USE app;\n"
        "CREATE TABLE users (id int, name string);\n"
        "INSERT INTO users (id, name) VALUE (1, \"Ann\");\n"
        "SELECT * FROM users;\n"
    );
    std::ostringstream output;
    std::ostringstream error;

    CliApp app(root);
    const int exitCode = app.runInteractive(input, output, error);
    assert(exitCode == 0);
    assert(error.str().empty());
    assert(output.str().find("[{\"id\":1,\"name\":\"Ann\"}]") != std::string::npos);

    std::filesystem::remove_all(root);
}

int main() {
    testBatchRunsCommandsAndPrintsResults();
    testBatchContinuesAfterErrors();
    testBatchReportsMissingFile();
    testInteractiveUsesPersistentEngine();
    return 0;
}

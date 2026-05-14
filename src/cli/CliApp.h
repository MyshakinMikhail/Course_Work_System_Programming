#pragma once

#include <filesystem>
#include <iosfwd>

#include "Engine.h"
#include "Parser.h"

class CliApp {
public:
    explicit CliApp(std::filesystem::path dataRoot = "data");

    int runInteractive(std::istream& input, std::ostream& output, std::ostream& error);
    int runBatch(const std::filesystem::path& scriptPath,
                 std::ostream& output,
                 std::ostream& error);

private:
    Parser parser_;
    Engine engine_;

    void executeCommand(const std::string& command,
                        std::ostream& output,
                        std::ostream& error);
};

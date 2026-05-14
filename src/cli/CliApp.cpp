#include "CliApp.h"

#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include "CommandReader.h"
#include "ParseError.h"

CliApp::CliApp(std::filesystem::path dataRoot)
    : engine_(std::move(dataRoot)) {}

int CliApp::runInteractive(std::istream& input, std::ostream& output, std::ostream& error) {
    CommandReader reader(input);
    std::string command;

    output << "> ";
    while (reader.readNext(command)) {
        executeCommand(command, output, error);
        output << "> ";
    }

    return 0;
}

int CliApp::runBatch(const std::filesystem::path& scriptPath,
                     std::ostream& output,
                     std::ostream& error) {
    std::ifstream input(scriptPath);
    if (!input.is_open()) {
        error << "Error: failed to open script file '" << scriptPath.string() << "'" << '\n';
        return 1;
    }

    CommandReader reader(input);
    std::string command;
    while (reader.readNext(command)) {
        executeCommand(command, output, error);
    }

    return 0;
}

void CliApp::executeCommand(const std::string& command,
                            std::ostream& output,
                            std::ostream& error) {
    try {
        const ExecutionResult result = engine_.execute(parser_.parse(command));
        if (result.isSuccess()) {
            if (!result.json.empty()) {
                output << result.json << '\n';
            } else if (!result.message.empty()) {
                output << result.message << '\n';
            }
        } else {
            error << "Error: " << result.message << '\n';
        }
    } catch (const ParseError& parseError) {
        error << "Error: " << parseError.what() << '\n';
    } catch (const std::exception& exception) {
        error << "Error: " << exception.what() << '\n';
    }
}

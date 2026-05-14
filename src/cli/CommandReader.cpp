#include "CommandReader.h"

#include <cctype>

namespace {

bool containsNonWhitespace(const std::string& value) {
    for (const char ch : value) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            return true;
        }
    }
    return false;
}

}

CommandReader::CommandReader(std::istream& input)
    : input_(input) {}

bool CommandReader::readNext(std::string& command) {
    command.clear();

    bool insideString = false;
    char ch = '\0';
    while (input_.get(ch)) {
        command.push_back(ch);

        if (ch == '"') {
            insideString = !insideString;
        }

        if (ch == ';' && !insideString) {
            return containsNonWhitespace(command);
        }
    }

    return containsNonWhitespace(command);
}

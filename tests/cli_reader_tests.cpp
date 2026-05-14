#include <cassert>
#include <sstream>
#include <string>

#include "CommandReader.h"

static void testReadsSingleCommand() {
    std::istringstream input("CREATE DATABASE app;");
    CommandReader reader(input);

    std::string command;
    assert(reader.readNext(command));
    assert(command == "CREATE DATABASE app;");
    assert(!reader.readNext(command));
}

static void testReadsMultipleCommands() {
    std::istringstream input("CREATE DATABASE app; USE app;\nSELECT * FROM users;");
    CommandReader reader(input);

    std::string command;
    assert(reader.readNext(command));
    assert(command == "CREATE DATABASE app;");

    assert(reader.readNext(command));
    assert(command == " USE app;");

    assert(reader.readNext(command));
    assert(command == "\nSELECT * FROM users;");

    assert(!reader.readNext(command));
}

static void testReadsMultilineCommand() {
    std::istringstream input("CREATE TABLE users (\nid int,\nname string\n);");
    CommandReader reader(input);

    std::string command;
    assert(reader.readNext(command));
    assert(command == "CREATE TABLE users (\nid int,\nname string\n);");
}

static void testIgnoresSemicolonInsideString() {
    std::istringstream input("INSERT INTO users (id, name) VALUE (1, \"A;B\"); SELECT * FROM users;");
    CommandReader reader(input);

    std::string command;
    assert(reader.readNext(command));
    assert(command == "INSERT INTO users (id, name) VALUE (1, \"A;B\");");

    assert(reader.readNext(command));
    assert(command == " SELECT * FROM users;");
}

static void testReturnsTrailingUnterminatedCommand() {
    std::istringstream input("USE app");
    CommandReader reader(input);

    std::string command;
    assert(reader.readNext(command));
    assert(command == "USE app");
    assert(!reader.readNext(command));
}

int main() {
    testReadsSingleCommand();
    testReadsMultipleCommands();
    testReadsMultilineCommand();
    testIgnoresSemicolonInsideString();
    testReturnsTrailingUnterminatedCommand();
    return 0;
}

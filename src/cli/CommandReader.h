#pragma once

#include <istream>
#include <string>

class CommandReader {
public:
    explicit CommandReader(std::istream& input);

    bool readNext(std::string& command);

private:
    std::istream& input_;
};

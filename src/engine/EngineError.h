#pragma once

#include <stdexcept>
#include <string>

class EngineError : public std::runtime_error {
public:
    explicit EngineError(const std::string& message)
        : std::runtime_error(message) {}
};

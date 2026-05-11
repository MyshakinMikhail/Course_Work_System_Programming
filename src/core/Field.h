#pragma once

#include <string>
#include <utility>

enum class FieldType {
    INT,
    STRING
};

struct Field {
    FieldType type{FieldType::INT};
    std::string stringValue{};
    int intValue{0};

    Field() = default;
    explicit Field(int value)
        : type(FieldType::INT), intValue(value) {}

    explicit Field(std::string value)
        : type(FieldType::STRING), stringValue(std::move(value)) {}

    Field(FieldType fieldType, int value)
        : type(fieldType), intValue(value) {}

    Field(FieldType fieldType, std::string value)
        : type(fieldType), stringValue(std::move(value)) {}

    static Field Int(int value) {
        return Field(value);
    }

    static Field String(std::string value) {
        return Field(std::move(value));
    }

    bool isInt() const {
        return type == FieldType::INT;
    }

    bool isString() const {
        return type == FieldType::STRING;
    }
};

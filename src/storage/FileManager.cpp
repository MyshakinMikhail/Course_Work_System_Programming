#include "FileManager.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

template <typename T>
void writeBinary(std::ostream& stream, const T& value) {
    stream.write(reinterpret_cast<const char*>(&value), static_cast<std::streamsize>(sizeof(T)));
    if (!stream) {
        throw std::runtime_error("Failed to write binary data");
    }
}

template <typename T>
T readBinary(std::istream& stream) {
    T value{};
    stream.read(reinterpret_cast<char*>(&value), static_cast<std::streamsize>(sizeof(T)));
    if (!stream) {
        throw std::runtime_error("Failed to read binary data");
    }
    return value;
}

void writeString(std::ostream& stream, const std::string& value) {
    const std::uint64_t size = static_cast<std::uint64_t>(value.size());
    writeBinary(stream, size);
    if (size != 0) {
        stream.write(value.data(), static_cast<std::streamsize>(size));
        if (!stream) {
            throw std::runtime_error("Failed to write string data");
        }
    }
}

std::string readString(std::istream& stream) {
    const std::uint64_t size = readBinary<std::uint64_t>(stream);
    std::string value(size, '\0');
    if (size != 0) {
        stream.read(value.data(), static_cast<std::streamsize>(size));
        if (!stream) {
            throw std::runtime_error("Failed to read string data");
        }
    }
    return value;
}

FieldType readFieldType(std::istream& stream) {
    const auto rawType = readBinary<std::int32_t>(stream);
    switch (rawType) {
    case static_cast<std::int32_t>(FieldType::INT):
        return FieldType::INT;
    case static_cast<std::int32_t>(FieldType::STRING):
        return FieldType::STRING;
    default:
        throw std::runtime_error("Unknown field type");
    }
}

void writeFieldType(std::ostream& stream, FieldType type) {
    const auto rawType = static_cast<std::int32_t>(type);
    writeBinary(stream, rawType);
}

Record readRecordPayload(std::istream& stream, const TableSchema& schema) {
    const auto fieldCount = readBinary<std::uint64_t>(stream);
    if (fieldCount != schema.columns.size()) {
        throw std::runtime_error("Stored record does not match schema");
    }

    Record record;
    record.fields.reserve(static_cast<std::size_t>(fieldCount));
    for (std::uint64_t i = 0; i < fieldCount; ++i) {
        const Column& column = schema.columns[static_cast<std::size_t>(i)];
        if (column.type == FieldType::INT) {
            const auto value = readBinary<std::int32_t>(stream);
            record.fields.emplace_back(Field::Int(static_cast<int>(value)));
        } else {
            record.fields.emplace_back(Field::String(readString(stream)));
        }
    }

    return record;
}

}

FileManager::FileManager(std::filesystem::path filePath)
    : filePath_(std::move(filePath)) {}

void FileManager::writeSchema(const TableSchema& schema) {
    std::ofstream stream(filePath_, std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) {
        throw std::runtime_error("Failed to open file for schema writing");
    }

    const std::uint64_t columnCount = static_cast<std::uint64_t>(schema.columns.size());
    writeBinary(stream, columnCount);
    for (const auto& column : schema.columns) {
        writeString(stream, column.name);
        writeFieldType(stream, column.type);
        const std::uint8_t indexed = column.indexed ? 1U : 0U;
        writeBinary(stream, indexed);
    }

    stream.flush();
    if (!stream) {
        throw std::runtime_error("Failed to flush schema to disk");
    }

    dataStart_ = stream.tellp();
    schema_ = schema;
}

TableSchema FileManager::readSchema() {
    std::ifstream stream(filePath_, std::ios::binary);
    if (!stream.is_open()) {
        throw std::runtime_error("Failed to open file for schema reading");
    }

    TableSchema schema;
    const auto columnCount = readBinary<std::uint64_t>(stream);
    schema.columns.reserve(static_cast<std::size_t>(columnCount));
    for (std::uint64_t i = 0; i < columnCount; ++i) {
        Column column;
        column.name = readString(stream);
        column.type = readFieldType(stream);
        const auto indexed = readBinary<std::uint8_t>(stream);
        column.indexed = indexed != 0;
        schema.columns.push_back(std::move(column));
    }

    dataStart_ = stream.tellg();
    schema_ = schema;
    return schema;
}

void FileManager::ensureSchemaLoaded() {
    if (!schema_.has_value()) {
        (void)readSchema();
    }
}

std::streampos FileManager::writeRecord(const Record& record) {
    ensureSchemaLoaded();

    const auto& schema = *schema_;
    if (record.fields.size() != schema.columns.size()) {
        throw std::runtime_error("Record field count does not match schema");
    }

    std::fstream stream(filePath_, std::ios::binary | std::ios::in | std::ios::out);
    if (!stream.is_open()) {
        std::ofstream createStream(filePath_, std::ios::binary | std::ios::app);
        if (!createStream.is_open()) {
            throw std::runtime_error("Failed to open file for record writing");
        }
        createStream.close();
        stream.open(filePath_, std::ios::binary | std::ios::in | std::ios::out);
        if (!stream.is_open()) {
            throw std::runtime_error("Failed to reopen file for record writing");
        }
    }

    stream.seekp(0, std::ios::end);
    const std::streampos offset = stream.tellp();
    if (offset == std::streampos(-1)) {
        throw std::runtime_error("Failed to determine record offset");
    }

    const std::uint64_t fieldCount = static_cast<std::uint64_t>(record.fields.size());
    writeBinary(stream, fieldCount);
    for (std::size_t i = 0; i < record.fields.size(); ++i) {
        const auto& field = record.fields[i];
        const auto& column = schema.columns[i];

        if (field.type != column.type) {
            throw std::runtime_error("Record field type does not match schema");
        }

        if (field.isInt()) {
            const std::int32_t value = static_cast<std::int32_t>(field.intValue);
            writeBinary(stream, value);
        } else {
            writeString(stream, field.stringValue);
        }
    }

    stream.flush();
    if (!stream) {
        throw std::runtime_error("Failed to flush record to disk");
    }

    return offset;
}

Record FileManager::readRecord(std::streampos offset) {
    ensureSchemaLoaded();

    if (offset < dataStart_) {
        throw std::runtime_error("Record offset points inside schema");
    }

    std::ifstream stream(filePath_, std::ios::binary);
    if (!stream.is_open()) {
        throw std::runtime_error("Failed to open file for record reading");
    }

    stream.seekg(offset);
    if (!stream) {
        throw std::runtime_error("Failed to seek to record offset");
    }

    const auto& schema = *schema_;
    return readRecordPayload(stream, schema);
}

std::vector<Record> FileManager::readAllRecords() {
    ensureSchemaLoaded();

    std::ifstream stream(filePath_, std::ios::binary);
    if (!stream.is_open()) {
        throw std::runtime_error("Failed to open file for record scan");
    }

    stream.seekg(0, std::ios::end);
    const std::streampos fileEnd = stream.tellg();
    if (fileEnd == std::streampos(-1)) {
        throw std::runtime_error("Failed to determine file size");
    }

    std::vector<Record> records;
    stream.seekg(dataStart_);
    if (!stream) {
        throw std::runtime_error("Failed to seek to data section");
    }

    const auto& schema = *schema_;
    while (stream.tellg() < fileEnd) {
        records.push_back(readRecordPayload(stream, schema));
    }

    return records;
}

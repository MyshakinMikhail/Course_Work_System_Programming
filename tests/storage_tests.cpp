#include <cassert>
#include <filesystem>
#include <string>

#include "FileManager.h"

static void testFileManagerRoundTrip() {
    const auto tempFile = std::filesystem::temp_directory_path() / "course_work_storage_test.bin";
    std::filesystem::remove(tempFile);

    const TableSchema schema{
        Column::Integer("id", true),
        Column::Text("name"),
        Column::Text("notes")
    };

    const Record first{
        Field::Int(1),
        Field::String("alice"),
        Field::String("first")
    };

    const Record second{
        Field::Int(2),
        Field::String("bob"),
        Field::String("second")
    };

    std::streampos firstOffset{};
    std::streampos secondOffset{};

    {
        FileManager writer(tempFile);
        writer.writeSchema(schema);
        firstOffset = writer.writeRecord(first);
        secondOffset = writer.writeRecord(second);

        assert(static_cast<std::streamoff>(secondOffset) > static_cast<std::streamoff>(firstOffset));
    }

    {
        FileManager reader(tempFile);
        const TableSchema loadedSchema = reader.readSchema();
        assert(loadedSchema.size() == 3);
        assert(loadedSchema.columns[0].name == "id");
        assert(loadedSchema.columns[0].indexed);
        assert(loadedSchema.columns[1].type == FieldType::STRING);

        const Record loadedFirst = reader.readRecord(firstOffset);
        assert(loadedFirst.fields.size() == 3);
        assert(loadedFirst.fields[0].intValue == 1);
        assert(loadedFirst.fields[1].stringValue == "alice");
        assert(loadedFirst.fields[2].stringValue == "first");

        const Record loadedSecond = reader.readRecord(secondOffset);
        assert(loadedSecond.fields.size() == 3);
        assert(loadedSecond.fields[0].intValue == 2);
        assert(loadedSecond.fields[1].stringValue == "bob");
        assert(loadedSecond.fields[2].stringValue == "second");
    }

    std::filesystem::remove(tempFile);
}

int main() {
    testFileManagerRoundTrip();
    return 0;
}

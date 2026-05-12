#include <cassert>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "FileManager.h"
#include "Index.h"
#include "Table.h"

namespace {

class RecordingIndex final : public Index {
public:
    void insert(int key, int offset) override {
        offsetsByKey[key] = offset;
        insertionOrder.push_back({key, offset});
    }

    std::optional<int> find(int key) override {
        const auto it = offsetsByKey.find(key);
        if (it == offsetsByKey.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    std::map<int, int> offsetsByKey;
    std::vector<std::pair<int, int>> insertionOrder;
};

std::filesystem::path makeTempFilePath(const std::string& fileName) {
    return std::filesystem::temp_directory_path() / fileName;
}

}

static void testTableCreationWritesSchema() {
    const auto tempFile = makeTempFilePath("course_work_core_storage_create_test.bin");
    std::filesystem::remove(tempFile);

    const TableSchema schema{
        Column::Integer("id", true),
        Column::Text("name")
    };

    Table table(tempFile, schema);
    assert(std::filesystem::exists(tempFile));

    FileManager reader(tempFile);
    const TableSchema loadedSchema = reader.readSchema();
    assert(loadedSchema.size() == 2);
    assert(loadedSchema.columns[0].name == "id");
    assert(loadedSchema.columns[0].indexed);
    assert(loadedSchema.columns[1].name == "name");
    assert(loadedSchema.columns[1].type == FieldType::STRING);

    std::filesystem::remove(tempFile);
}

static void testInsertPassesGrowingOffsetsToIndex() {
    const auto tempFile = makeTempFilePath("course_work_core_storage_offset_test.bin");
    std::filesystem::remove(tempFile);

    Table table(tempFile, TableSchema{
        Column::Integer("id", true),
        Column::Text("name")
    });

    RecordingIndex index;
    table.index = &index;

    table.insert(Record{Field::Int(10), Field::String("alpha")});
    table.insert(Record{Field::Int(20), Field::String("beta")});

    assert(index.insertionOrder.size() == 2);
    assert(index.insertionOrder[0].first == 10);
    assert(index.insertionOrder[1].first == 20);
    assert(index.insertionOrder[0].second >= 0);
    assert(index.insertionOrder[1].second > index.insertionOrder[0].second);

    FileManager reader(tempFile);
    reader.readSchema();
    const Record first = reader.readRecord(static_cast<std::streampos>(index.insertionOrder[0].second));
    const Record second = reader.readRecord(static_cast<std::streampos>(index.insertionOrder[1].second));

    assert(first.fields[0].intValue == 10);
    assert(first.fields[1].stringValue == "alpha");
    assert(second.fields[0].intValue == 20);
    assert(second.fields[1].stringValue == "beta");

    std::filesystem::remove(tempFile);
}

static void testScanReadsAllInsertedRecords() {
    const auto tempFile = makeTempFilePath("course_work_core_storage_scan_test.bin");
    std::filesystem::remove(tempFile);

    Table table(tempFile, TableSchema{
        Column::Integer("id", true),
        Column::Text("name"),
        Column::Text("note")
    });

    table.insert(Record{Field::Int(1), Field::String("alice"), Field::String("first")});
    table.insert(Record{Field::Int(2), Field::String("bob"), Field::String("second")});
    table.insert(Record{Field::Int(3), Field::String("carol"), Field::String("third")});

    const std::vector<Record> records = table.scan();
    assert(records.size() == 3);

    assert(records[0].fields[0].intValue == 1);
    assert(records[0].fields[1].stringValue == "alice");
    assert(records[1].fields[0].intValue == 2);
    assert(records[1].fields[2].stringValue == "second");
    assert(records[2].fields[0].intValue == 3);
    assert(records[2].fields[1].stringValue == "carol");

    std::filesystem::remove(tempFile);
}

int main() {
    testTableCreationWritesSchema();
    testInsertPassesGrowingOffsetsToIndex();
    testScanReadsAllInsertedRecords();
    return 0;
}

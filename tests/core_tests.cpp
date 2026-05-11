#include <cassert>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "Field.h"
#include "Record.h"
#include "Table.h"
#include "TableSchema.h"

namespace {

template <typename Fn>
void expectRuntimeError(Fn&& fn) {
    bool thrown = false;
    try {
        fn();
    } catch (const std::runtime_error&) {
        thrown = true;
    }
    assert(thrown);
}

class MockTable final : public Table {
public:
    void insert(const Record& record) override {
        insertedRecords.push_back(record);
    }

    std::vector<Record> scan() override {
        return insertedRecords;
    }

    std::optional<Record> findByIndex(int key) override {
        if (key < 0 || static_cast<std::size_t>(key) >= insertedRecords.size()) {
            return std::nullopt;
        }
        return insertedRecords[static_cast<std::size_t>(key)];
    }

    std::vector<Record> insertedRecords;
};

}

static void testField() {
    const Field defaultField;
    assert(defaultField.type == FieldType::INT);
    assert(defaultField.intValue == 0);
    assert(defaultField.stringValue.empty());

    const Field intField = Field::Int(42);
    assert(intField.isInt());
    assert(intField.type == FieldType::INT);
    assert(intField.intValue == 42);

    const Field stringField = Field::String("hello");
    assert(stringField.isString());
    assert(stringField.type == FieldType::STRING);
    assert(stringField.stringValue == "hello");

    const Field directInt(FieldType::INT, 7);
    assert(directInt.isInt());
    assert(directInt.intValue == 7);

    const Field directString(FieldType::STRING, "world");
    assert(directString.isString());
    assert(directString.stringValue == "world");
}

static void testRecord() {
    const Record emptyRecord;
    assert(emptyRecord.empty());
    assert(emptyRecord.size() == 0);

    Record record{Field::Int(1), Field::String("two")};
    assert(record.size() == 2);
    assert(!record.empty());
    assert(record.fields[0].intValue == 1);
    assert(record.fields[1].stringValue == "two");

    record.addField(Field::Int(3));
    assert(record.size() == 3);
    assert(record.fields[2].intValue == 3);

    const Record vectorRecord(std::vector<Field>{Field::Int(10), Field::String("ten")});
    assert(vectorRecord.size() == 2);
    assert(vectorRecord.fields[0].intValue == 10);
    assert(vectorRecord.fields[1].stringValue == "ten");
}

static void testTableSchema() {
    const TableSchema emptySchema;
    assert(emptySchema.empty());
    assert(emptySchema.size() == 0);

    TableSchema schema{
        Column::Integer("id", true),
        Column::Text("name")
    };

    assert(schema.size() == 2);
    assert(!schema.empty());

    const Column* idColumn = schema.findColumn("id");
    assert(idColumn != nullptr);
    assert(idColumn->type == FieldType::INT);
    assert(idColumn->indexed);

    const Column* nameColumn = schema.findColumn("name");
    assert(nameColumn != nullptr);
    assert(nameColumn->type == FieldType::STRING);
    assert(!nameColumn->indexed);

    const Column* missingColumn = schema.findColumn("missing");
    assert(missingColumn == nullptr);

    const Column notesColumn("notes", FieldType::STRING);
    assert(notesColumn.name == "notes");
    assert(notesColumn.type == FieldType::STRING);
    assert(!notesColumn.indexed);
}

static void testTableContract() {
    static_assert(!std::is_abstract_v<Table>, "Table must be concrete");

    MockTable table;
    const Record first{Field::Int(1), Field::String("one")};
    const Record second{Field::Int(2), Field::String("two")};

    table.insert(first);
    table.insert(second);

    assert(table.insertedRecords.size() == 2);
    assert(table.insertedRecords[0].fields[0].intValue == 1);
    assert(table.insertedRecords[1].fields[1].stringValue == "two");

    const std::vector<Record> snapshot = table.scan();
    assert(snapshot.size() == 2);
    assert(snapshot[0].fields[0].intValue == 1);
    assert(snapshot[1].fields[0].intValue == 2);

    const std::optional<Record> found = table.findByIndex(1);
    assert(found.has_value());
    assert(found->fields[0].intValue == 2);

    const std::optional<Record> missing = table.findByIndex(5);
    assert(!missing.has_value());
}

static void testTableStorageIntegration() {
    const auto tempFile = std::filesystem::temp_directory_path() / "course_work_table_test.bin";
    std::filesystem::remove(tempFile);

    Table table(tempFile, TableSchema{
        Column::Integer("id", true),
        Column::Text("name")
    });

    table.insert(Record{Field::Int(11), Field::String("alpha")});
    table.insert(Record{Field::Int(22), Field::String("beta")});

    const std::vector<Record> snapshot = table.scan();
    assert(snapshot.size() == 2);
    assert(snapshot[0].fields[0].intValue == 11);
    assert(snapshot[1].fields[1].stringValue == "beta");

    const std::optional<Record> found = table.findByIndex(1);
    assert(found.has_value());
    assert(found->fields[0].intValue == 22);

    std::filesystem::remove(tempFile);
}

static void testTableRequiresInitialization() {
    Table table;

    expectRuntimeError([&]() {
        table.insert(Record{Field::Int(1)});
    });

    expectRuntimeError([&]() {
        (void)table.scan();
    });

    expectRuntimeError([&]() {
        (void)table.findByIndex(0);
    });
}

int main() {
    testField();
    testRecord();
    testTableSchema();
    testTableContract();
    testTableRequiresInitialization();
    testTableStorageIntegration();
    return 0;
}

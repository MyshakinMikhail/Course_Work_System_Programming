#include <cassert>
#include <string>
#include <vector>

#include "Field.h"
#include "Record.h"
#include "TableSchema.h"

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

int main() {
    testField();
    testRecord();
    testTableSchema();
    return 0;
}

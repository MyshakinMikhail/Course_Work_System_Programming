#pragma once

#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

#include "Field.h"

struct Record {
    std::vector<Field> fields;

    Record() = default;
    explicit Record(std::vector<Field> values)
        : fields(std::move(values)) {}

    Record(std::initializer_list<Field> values)
        : fields(values) {}

    void addField(const Field& field) {
        fields.push_back(field);
    }

    std::size_t size() const {
        return fields.size();
    }

    bool empty() const {
        return fields.empty();
    }
};

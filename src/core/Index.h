#pragma once

#include <optional>

class Index {
public:
    virtual ~Index() = default;

    virtual void insert(int key, int offset) = 0;
    virtual std::optional<int> find(int key) = 0;
};

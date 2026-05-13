#pragma once

#include <string>
#include <utility>

enum class ExecutionStatus {
    Success,
    Error
};

struct ExecutionResult {
    ExecutionStatus status{ExecutionStatus::Success};
    std::string message;
    std::string json;

    static ExecutionResult Success(std::string message = {}) {
        ExecutionResult result;
        result.status = ExecutionStatus::Success;
        result.message = std::move(message);
        return result;
    }

    static ExecutionResult Json(std::string json) {
        ExecutionResult result;
        result.status = ExecutionStatus::Success;
        result.json = std::move(json);
        return result;
    }

    static ExecutionResult Error(std::string message) {
        ExecutionResult result;
        result.status = ExecutionStatus::Error;
        result.message = std::move(message);
        return result;
    }

    bool isSuccess() const {
        return status == ExecutionStatus::Success;
    }
};

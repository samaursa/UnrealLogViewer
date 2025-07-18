#pragma once

#include "../../macros.h"
#include "filter.h"
#include "../log_parser/log_entry.h"
#include <memory>
#include <vector>
#include <string>

namespace ue_log {

enum class FilterOperator {
    And,
    Or
};

enum class FilterConditionType {
    MessageContains,
    MessageEquals,
    MessageRegex,
    LoggerEquals,
    LoggerContains,
    LogLevelEquals,
    TimestampAfter,
    TimestampBefore,
    TimestampEquals,
    FrameAfter,
    FrameBefore,
    FrameEquals,
    AnyFieldContains
};

/**
 * Represents a single filter condition (leaf node in the expression tree)
 */
class FilterCondition {
    CK_GENERATED_BODY(FilterCondition);
    
private:
    FilterConditionType type_;
    std::string value_;
    bool is_active_;
    
public:
    FilterCondition(FilterConditionType type, const std::string& value)
        : type_(type), value_(value), is_active_(true) {}
    
    // Properties
    CK_PROPERTY(type_);
    CK_PROPERTY(value_);
    CK_PROPERTY(is_active_);
    
    // Evaluation
    bool Matches(const LogEntry& entry) const;
    
    // Display
    std::string ToString() const;
    std::string GetFieldName() const;
    std::string GetOperatorName() const;
};

/**
 * Represents a hierarchical filter expression tree
 */
class FilterExpression {
    CK_GENERATED_BODY(FilterExpression);
    
private:
    FilterOperator operator_;
    std::vector<std::unique_ptr<FilterCondition>> conditions_;
    std::vector<std::unique_ptr<FilterExpression>> sub_expressions_;
    bool is_active_;
    
public:
    FilterExpression(FilterOperator op = FilterOperator::And)
        : operator_(op), is_active_(true) {}
    
    // Properties
    CK_PROPERTY(operator_);
    CK_PROPERTY(is_active_);
    
    // Condition management
    void AddCondition(std::unique_ptr<FilterCondition> condition);
    void AddSubExpression(std::unique_ptr<FilterExpression> expression);
    void RemoveCondition(size_t index);
    void RemoveSubExpression(size_t index);
    
    // Access
    const std::vector<std::unique_ptr<FilterCondition>>& GetConditions() const { return conditions_; }
    const std::vector<std::unique_ptr<FilterExpression>>& GetSubExpressions() const { return sub_expressions_; }
    
    // Evaluation
    bool Matches(const LogEntry& entry) const;
    
    // Display
    std::string ToString() const;
    bool IsEmpty() const;
    
    // Utility
    void Clear();
    size_t GetTotalConditions() const;
};

/**
 * Factory methods for creating common filter conditions
 */
class FilterConditionFactory {
public:
    static std::unique_ptr<FilterCondition> CreateMessageContains(const std::string& text);
    static std::unique_ptr<FilterCondition> CreateLoggerEquals(const std::string& logger);
    static std::unique_ptr<FilterCondition> CreateLogLevelEquals(const std::string& level);
    static std::unique_ptr<FilterCondition> CreateTimestampAfter(const std::string& timestamp);
    static std::unique_ptr<FilterCondition> CreateTimestampBefore(const std::string& timestamp);
    static std::unique_ptr<FilterCondition> CreateFrameAfter(int frame);
    static std::unique_ptr<FilterCondition> CreateFrameBefore(int frame);
    static std::unique_ptr<FilterCondition> CreateAnyFieldContains(const std::string& text);
    
    // Contextual filter creation from log entry
    static std::vector<std::unique_ptr<FilterCondition>> CreateContextualConditions(const LogEntry& entry);
};

} // namespace ue_log
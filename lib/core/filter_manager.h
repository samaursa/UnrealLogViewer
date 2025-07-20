#pragma once

#include "../log_parser/log_entry.h"
#include "../filter_engine/filter_expression.h"
#include <memory>
#include <vector>
#include <string>

namespace ue_log {

/**
 * Core business logic for filter management without UI dependencies.
 * Handles filter creation, application, and management in a headless manner
 * suitable for automated testing.
 */
class FilterManager {
public:
    FilterManager();
    ~FilterManager();
    
    // Quick filter operations
    void AddQuickFilter(const std::string& filter_type);
    void ClearAllFilters();
    bool HasActiveFilters() const;
    size_t GetActiveFilterCount() const;
    std::vector<std::string> GetActiveFilterDescriptions() const;
    
    // Filter application
    std::vector<LogEntry> ApplyFilters(const std::vector<LogEntry>& entries) const;
    bool ShouldIncludeEntry(const LogEntry& entry) const;
    
    // Advanced filter operations
    void AddCustomFilter(std::unique_ptr<FilterCondition> condition);
    void AddFilterExpression(std::unique_ptr<FilterExpression> expression);
    void RemoveFilter(size_t index);
    void ToggleFilter(size_t index);
    void SetFilterActive(size_t index, bool active);
    
    // Filter inspection
    const std::vector<std::unique_ptr<FilterCondition>>& GetActiveConditions() const;
    const std::vector<std::unique_ptr<FilterExpression>>& GetActiveExpressions() const;
    
    // Statistics
    struct FilterStats {
        size_t total_entries;
        size_t filtered_entries;
        size_t excluded_entries;
        double filter_ratio;
    };
    FilterStats GetFilterStats(const std::vector<LogEntry>& entries) const;
    
    // Error handling
    std::string GetLastError() const;
    
private:
    std::vector<std::unique_ptr<FilterCondition>> active_conditions_;
    std::vector<std::unique_ptr<FilterExpression>> active_expressions_;
    std::string last_error_;
    
    // Helper methods
    std::unique_ptr<FilterCondition> CreateQuickFilterCondition(const std::string& filter_type);
    void ClearError();
    void SetError(const std::string& error);
};

} // namespace ue_log
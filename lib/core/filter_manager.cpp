#include "filter_manager.h"
#include <algorithm>

namespace ue_log {

FilterManager::FilterManager() = default;

FilterManager::~FilterManager() = default;

void FilterManager::AddQuickFilter(const std::string& filter_type) {
    ClearError();
    
    auto condition = CreateQuickFilterCondition(filter_type);
    if (condition) {
        active_conditions_.push_back(std::move(condition));
    } else {
        SetError("Unknown filter type: " + filter_type);
    }
}

void FilterManager::ClearAllFilters() {
    active_conditions_.clear();
    active_expressions_.clear();
    ClearError();
}

bool FilterManager::HasActiveFilters() const {
    return !active_conditions_.empty() || !active_expressions_.empty();
}

size_t FilterManager::GetActiveFilterCount() const {
    return active_conditions_.size() + active_expressions_.size();
}

std::vector<std::string> FilterManager::GetActiveFilterDescriptions() const {
    std::vector<std::string> descriptions;
    
    // Add condition descriptions
    for (const auto& condition : active_conditions_) {
        if (condition && condition->Get_is_active_()) {
            descriptions.push_back(condition->ToString());
        }
    }
    
    // Add expression descriptions
    for (const auto& expression : active_expressions_) {
        if (expression && expression->Get_is_active_()) {
            descriptions.push_back(expression->ToString());
        }
    }
    
    return descriptions;
}

std::vector<LogEntry> FilterManager::ApplyFilters(const std::vector<LogEntry>& entries) const {
    if (!HasActiveFilters()) {
        return entries; // No filters, return all entries
    }
    
    std::vector<LogEntry> filtered_entries;
    filtered_entries.reserve(entries.size());
    
    for (const auto& entry : entries) {
        if (ShouldIncludeEntry(entry)) {
            filtered_entries.push_back(entry);
        }
    }
    
    return filtered_entries;
}

bool FilterManager::ShouldIncludeEntry(const LogEntry& entry) const {
    if (!HasActiveFilters()) {
        return true; // No filters, include all entries
    }
    
    // Check all active conditions (AND logic)
    for (const auto& condition : active_conditions_) {
        if (condition && condition->Get_is_active_()) {
            if (!condition->Matches(entry)) {
                return false; // Condition failed, exclude entry
            }
        }
    }
    
    // Check all active expressions (AND logic)
    for (const auto& expression : active_expressions_) {
        if (expression && expression->Get_is_active_()) {
            if (!expression->Matches(entry)) {
                return false; // Expression failed, exclude entry
            }
        }
    }
    
    return true; // All conditions passed, include entry
}

void FilterManager::AddCustomFilter(std::unique_ptr<FilterCondition> condition) {
    if (condition) {
        active_conditions_.push_back(std::move(condition));
        ClearError();
    } else {
        SetError("Cannot add null filter condition");
    }
}

void FilterManager::AddFilterExpression(std::unique_ptr<FilterExpression> expression) {
    if (expression) {
        active_expressions_.push_back(std::move(expression));
        ClearError();
    } else {
        SetError("Cannot add null filter expression");
    }
}

void FilterManager::RemoveFilter(size_t index) {
    size_t condition_count = active_conditions_.size();
    
    if (index < condition_count) {
        // Remove from conditions
        active_conditions_.erase(active_conditions_.begin() + index);
        ClearError();
    } else if (index < condition_count + active_expressions_.size()) {
        // Remove from expressions
        size_t expr_index = index - condition_count;
        active_expressions_.erase(active_expressions_.begin() + expr_index);
        ClearError();
    } else {
        SetError("Filter index out of range");
    }
}

void FilterManager::ToggleFilter(size_t index) {
    size_t condition_count = active_conditions_.size();
    
    if (index < condition_count) {
        // Toggle condition
        auto& condition = active_conditions_[index];
        if (condition) {
            condition->Request_is_active_(!condition->Get_is_active_());
            ClearError();
        }
    } else if (index < condition_count + active_expressions_.size()) {
        // Toggle expression
        size_t expr_index = index - condition_count;
        auto& expression = active_expressions_[expr_index];
        if (expression) {
            expression->Request_is_active_(!expression->Get_is_active_());
            ClearError();
        }
    } else {
        SetError("Filter index out of range");
    }
}

void FilterManager::SetFilterActive(size_t index, bool active) {
    size_t condition_count = active_conditions_.size();
    
    if (index < condition_count) {
        // Set condition active state
        auto& condition = active_conditions_[index];
        if (condition) {
            condition->Request_is_active_(active);
            ClearError();
        }
    } else if (index < condition_count + active_expressions_.size()) {
        // Set expression active state
        size_t expr_index = index - condition_count;
        auto& expression = active_expressions_[expr_index];
        if (expression) {
            expression->Request_is_active_(active);
            ClearError();
        }
    } else {
        SetError("Filter index out of range");
    }
}

const std::vector<std::unique_ptr<FilterCondition>>& FilterManager::GetActiveConditions() const {
    return active_conditions_;
}

const std::vector<std::unique_ptr<FilterExpression>>& FilterManager::GetActiveExpressions() const {
    return active_expressions_;
}

FilterManager::FilterStats FilterManager::GetFilterStats(const std::vector<LogEntry>& entries) const {
    FilterStats stats;
    stats.total_entries = entries.size();
    
    if (!HasActiveFilters()) {
        stats.filtered_entries = stats.total_entries;
        stats.excluded_entries = 0;
        stats.filter_ratio = 1.0;
    } else {
        auto filtered = ApplyFilters(entries);
        stats.filtered_entries = filtered.size();
        stats.excluded_entries = stats.total_entries - stats.filtered_entries;
        stats.filter_ratio = stats.total_entries > 0 ? 
            static_cast<double>(stats.filtered_entries) / static_cast<double>(stats.total_entries) : 0.0;
    }
    
    return stats;
}

std::string FilterManager::GetLastError() const {
    return last_error_;
}

std::unique_ptr<FilterCondition> FilterManager::CreateQuickFilterCondition(const std::string& filter_type) {
    if (filter_type == "error") {
        return std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Error");
    } else if (filter_type == "warning") {
        return std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Warning");
    } else if (filter_type == "info") {
        return std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Info");
    } else if (filter_type == "debug") {
        return std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, "Debug");
    } else if (filter_type == "clear") {
        // Special case: clear all filters
        ClearAllFilters();
        return nullptr; // Don't add a new condition
    }
    
    return nullptr; // Unknown filter type
}

void FilterManager::ClearError() {
    last_error_.clear();
}

void FilterManager::SetError(const std::string& error) {
    last_error_ = error;
}

} // namespace ue_log
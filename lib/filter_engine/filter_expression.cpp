#include "filter_expression.h"
#include <algorithm>
#include <regex>
#include <sstream>

namespace ue_log {

// FilterCondition implementation

bool FilterCondition::Matches(const LogEntry& entry) const {
    if (!is_active_) {
        return true; // Inactive conditions don't filter anything
    }
    
    switch (type_) {
        case FilterConditionType::MessageContains: {
            std::string message = entry.Get_message();
            std::string value = value_;
            std::transform(message.begin(), message.end(), message.begin(), ::tolower);
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            return message.find(value) != std::string::npos;
        }
        
        case FilterConditionType::MessageEquals: {
            return entry.Get_message() == value_;
        }
        
        case FilterConditionType::MessageRegex: {
            try {
                std::regex pattern(value_, std::regex_constants::icase);
                return std::regex_search(entry.Get_message(), pattern);
            } catch (const std::regex_error&) {
                return false; // Invalid regex doesn't match anything
            }
        }
        
        case FilterConditionType::LoggerEquals: {
            return entry.Get_logger_name() == value_;
        }
        
        case FilterConditionType::LoggerContains: {
            std::string logger = entry.Get_logger_name();
            std::string value = value_;
            std::transform(logger.begin(), logger.end(), logger.begin(), ::tolower);
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            return logger.find(value) != std::string::npos;
        }
        
        case FilterConditionType::LogLevelEquals: {
            if (!entry.Get_log_level().has_value()) {
                return false;
            }
            return entry.Get_log_level().value() == value_;
        }
        
        case FilterConditionType::TimestampAfter: {
            if (!entry.Get_timestamp().has_value()) {
                return false;
            }
            return entry.Get_timestamp().value() >= value_;
        }
        
        case FilterConditionType::TimestampBefore: {
            if (!entry.Get_timestamp().has_value()) {
                return false;
            }
            return entry.Get_timestamp().value() <= value_;
        }
        
        case FilterConditionType::TimestampEquals: {
            if (!entry.Get_timestamp().has_value()) {
                return false;
            }
            return entry.Get_timestamp().value() == value_;
        }
        
        case FilterConditionType::FrameAfter: {
            if (!entry.Get_frame_number().has_value()) {
                return false;
            }
            try {
                int frame_value = std::stoi(value_);
                return entry.Get_frame_number().value() >= frame_value;
            } catch (const std::exception&) {
                return false;
            }
        }
        
        case FilterConditionType::FrameBefore: {
            if (!entry.Get_frame_number().has_value()) {
                return false;
            }
            try {
                int frame_value = std::stoi(value_);
                return entry.Get_frame_number().value() <= frame_value;
            } catch (const std::exception&) {
                return false;
            }
        }
        
        case FilterConditionType::FrameEquals: {
            if (!entry.Get_frame_number().has_value()) {
                return false;
            }
            try {
                int frame_value = std::stoi(value_);
                return entry.Get_frame_number().value() == frame_value;
            } catch (const std::exception&) {
                return false;
            }
        }
        
        case FilterConditionType::LineAfter: {
            try {
                int line_value = std::stoi(value_);
                return static_cast<int>(entry.Get_line_number()) >= line_value;
            } catch (const std::exception&) {
                return false;
            }
        }
        
        case FilterConditionType::LineBefore: {
            try {
                int line_value = std::stoi(value_);
                return static_cast<int>(entry.Get_line_number()) < line_value;
            } catch (const std::exception&) {
                return false;
            }
        }
        
        case FilterConditionType::AnyFieldContains: {
            std::string search_text = entry.Get_message() + " " + entry.Get_logger_name();
            if (entry.Get_log_level().has_value()) {
                search_text += " " + entry.Get_log_level().value();
            }
            if (entry.Get_timestamp().has_value()) {
                search_text += " " + entry.Get_timestamp().value();
            }
            
            std::string value = value_;
            std::transform(search_text.begin(), search_text.end(), search_text.begin(), ::tolower);
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            return search_text.find(value) != std::string::npos;
        }
        
        default:
            return false;
    }
}

std::string FilterCondition::ToString() const {
    std::string field = GetFieldName();
    std::string op = GetOperatorName();
    return field + " " + op + " \"" + value_ + "\"";
}

std::string FilterCondition::GetFieldName() const {
    switch (type_) {
        case FilterConditionType::MessageContains:
        case FilterConditionType::MessageEquals:
        case FilterConditionType::MessageRegex:
            return "Message";
        case FilterConditionType::LoggerEquals:
        case FilterConditionType::LoggerContains:
            return "Logger";
        case FilterConditionType::LogLevelEquals:
            return "LogLevel";
        case FilterConditionType::TimestampAfter:
        case FilterConditionType::TimestampBefore:
        case FilterConditionType::TimestampEquals:
            return "Timestamp";
        case FilterConditionType::FrameAfter:
        case FilterConditionType::FrameBefore:
        case FilterConditionType::FrameEquals:
            return "Frame";
        case FilterConditionType::LineAfter:
        case FilterConditionType::LineBefore:
            return "Line";
        case FilterConditionType::AnyFieldContains:
            return "Any";
        default:
            return "Unknown";
    }
}

std::string FilterCondition::GetOperatorName() const {
    switch (type_) {
        case FilterConditionType::MessageContains:
        case FilterConditionType::LoggerContains:
        case FilterConditionType::AnyFieldContains:
            return "contains";
        case FilterConditionType::MessageEquals:
        case FilterConditionType::LoggerEquals:
        case FilterConditionType::LogLevelEquals:
        case FilterConditionType::TimestampEquals:
        case FilterConditionType::FrameEquals:
            return "=";
        case FilterConditionType::MessageRegex:
            return "matches";
        case FilterConditionType::TimestampAfter:
        case FilterConditionType::FrameAfter:
        case FilterConditionType::LineAfter:
            return ">=";
        case FilterConditionType::TimestampBefore:
        case FilterConditionType::FrameBefore:
        case FilterConditionType::LineBefore:
            return "<";
        default:
            return "?";
    }
}

// FilterExpression implementation

void FilterExpression::AddCondition(std::unique_ptr<FilterCondition> condition) {
    conditions_.push_back(std::move(condition));
}

void FilterExpression::AddSubExpression(std::unique_ptr<FilterExpression> expression) {
    sub_expressions_.push_back(std::move(expression));
}

void FilterExpression::RemoveCondition(size_t index) {
    if (index < conditions_.size()) {
        conditions_.erase(conditions_.begin() + index);
    }
}

void FilterExpression::RemoveSubExpression(size_t index) {
    if (index < sub_expressions_.size()) {
        sub_expressions_.erase(sub_expressions_.begin() + index);
    }
}

bool FilterExpression::Matches(const LogEntry& entry) const {
    if (!is_active_) {
        return true; // Inactive expressions don't filter anything
    }
    
    if (IsEmpty()) {
        return true; // Empty expressions match everything
    }
    
    bool result = (operator_ == FilterOperator::And);
    
    // Evaluate conditions
    for (const auto& condition : conditions_) {
        bool condition_result = condition->Matches(entry);
        
        if (operator_ == FilterOperator::And) {
            result = result && condition_result;
            if (!result) break; // Short-circuit AND
        } else { // OR
            result = result || condition_result;
            if (result) break; // Short-circuit OR
        }
    }
    
    // Evaluate sub-expressions
    for (const auto& sub_expr : sub_expressions_) {
        bool sub_result = sub_expr->Matches(entry);
        
        if (operator_ == FilterOperator::And) {
            result = result && sub_result;
            if (!result) break; // Short-circuit AND
        } else { // OR
            result = result || sub_result;
            if (result) break; // Short-circuit OR
        }
    }
    
    return result;
}

std::string FilterExpression::ToString() const {
    if (IsEmpty()) {
        return "No filters";
    }
    
    std::vector<std::string> parts;
    
    // Add conditions
    for (const auto& condition : conditions_) {
        if (condition->Get_is_active_()) {
            parts.push_back(condition->ToString());
        }
    }
    
    // Add sub-expressions
    for (const auto& sub_expr : sub_expressions_) {
        if (sub_expr->Get_is_active_() && !sub_expr->IsEmpty()) {
            parts.push_back("(" + sub_expr->ToString() + ")");
        }
    }
    
    if (parts.empty()) {
        return "No active filters";
    }
    
    std::string op_str = (operator_ == FilterOperator::And) ? " AND " : " OR ";
    std::ostringstream result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            result << op_str;
        }
        result << parts[i];
    }
    
    return result.str();
}

bool FilterExpression::IsEmpty() const {
    return conditions_.empty() && sub_expressions_.empty();
}

void FilterExpression::Clear() {
    conditions_.clear();
    sub_expressions_.clear();
}

size_t FilterExpression::GetTotalConditions() const {
    size_t count = conditions_.size();
    for (const auto& sub_expr : sub_expressions_) {
        count += sub_expr->GetTotalConditions();
    }
    return count;
}

// FilterConditionFactory implementation

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateMessageContains(const std::string& text) {
    return std::make_unique<FilterCondition>(FilterConditionType::MessageContains, text);
}

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateLoggerEquals(const std::string& logger) {
    return std::make_unique<FilterCondition>(FilterConditionType::LoggerEquals, logger);
}

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateLogLevelEquals(const std::string& level) {
    return std::make_unique<FilterCondition>(FilterConditionType::LogLevelEquals, level);
}

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateTimestampAfter(const std::string& timestamp) {
    return std::make_unique<FilterCondition>(FilterConditionType::TimestampAfter, timestamp);
}

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateTimestampBefore(const std::string& timestamp) {
    return std::make_unique<FilterCondition>(FilterConditionType::TimestampBefore, timestamp);
}

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateFrameAfter(int frame) {
    return std::make_unique<FilterCondition>(FilterConditionType::FrameAfter, std::to_string(frame));
}

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateFrameBefore(int frame) {
    return std::make_unique<FilterCondition>(FilterConditionType::FrameBefore, std::to_string(frame));
}

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateLineAfter(int line) {
    return std::make_unique<FilterCondition>(FilterConditionType::LineAfter, std::to_string(line));
}

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateLineBefore(int line) {
    return std::make_unique<FilterCondition>(FilterConditionType::LineBefore, std::to_string(line));
}

std::unique_ptr<FilterCondition> FilterConditionFactory::CreateAnyFieldContains(const std::string& text) {
    return std::make_unique<FilterCondition>(FilterConditionType::AnyFieldContains, text);
}

std::vector<std::unique_ptr<FilterCondition>> FilterConditionFactory::CreateContextualConditions(const LogEntry& entry) {
    std::vector<std::unique_ptr<FilterCondition>> conditions;
    
    // Timestamp-based conditions
    if (entry.Get_timestamp().has_value()) {
        conditions.push_back(CreateTimestampAfter(entry.Get_timestamp().value()));
        conditions.push_back(CreateTimestampBefore(entry.Get_timestamp().value()));
    }
    
    // Frame-based conditions
    if (entry.Get_frame_number().has_value()) {
        conditions.push_back(CreateFrameAfter(entry.Get_frame_number().value()));
        conditions.push_back(CreateFrameBefore(entry.Get_frame_number().value()));
    }
    
    // Logger-based conditions
    if (!entry.Get_logger_name().empty()) {
        conditions.push_back(CreateLoggerEquals(entry.Get_logger_name()));
    }
    
    // Log level-based conditions
    if (entry.Get_log_level().has_value()) {
        conditions.push_back(CreateLogLevelEquals(entry.Get_log_level().value()));
    }
    
    // Message-based conditions
    if (!entry.Get_message().empty()) {
        conditions.push_back(CreateMessageContains(entry.Get_message()));
    }
    
    return conditions;
}

} // namespace ue_log
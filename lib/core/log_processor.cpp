#include "log_processor.h"
#include <filesystem>
#include <algorithm>
#include <regex>

namespace ue_log {

LogProcessor::LogProcessor() 
    : log_parser_(std::make_unique<LogParser>())
    , current_index_(0)
    , current_search_index_(-1) {
}

LogProcessor::~LogProcessor() = default;

bool LogProcessor::LoadFile(const std::string& file_path) {
    if (file_path.empty()) {
        last_error_ = "File path is empty";
        return false;
    }
    
    // Check if file exists
    if (!std::filesystem::exists(file_path)) {
        last_error_ = "File does not exist: " + file_path;
        return false;
    }
    
    try {
        // Use LogParser to load and parse the file
        auto load_result = log_parser_->LoadFile(file_path);
        if (load_result.IsError()) {
            last_error_ = "Failed to load file: " + file_path + " - " + load_result.Get_error_message();
            return false;
        }
        
        // Parse the log entries
        log_entries_ = log_parser_->ParseEntries();
        
        if (log_entries_.empty()) {
            last_error_ = "No valid log entries found in file";
            return false;
        }
        
        // Initialize filtered entries to all entries
        filtered_entries_ = log_entries_;
        
        // Store the file path and reset state
        current_file_path_ = file_path;
        current_index_ = 0;
        ClearSearchResults();
        last_error_.clear();
        
        return true;
        
    } catch (const std::exception& e) {
        last_error_ = "Error loading file: " + std::string(e.what());
        return false;
    }
}

void LogProcessor::ClearEntries() {
    log_entries_.clear();
    filtered_entries_.clear();
    current_file_path_.clear();
    current_index_ = 0;
    ClearSearchResults();
    last_error_.clear();
}

bool LogProcessor::IsFileLoaded() const {
    return !current_file_path_.empty() && !log_entries_.empty();
}

std::string LogProcessor::GetCurrentFilePath() const {
    return current_file_path_;
}

std::string LogProcessor::GetLastError() const {
    return last_error_;
}

const std::vector<LogEntry>& LogProcessor::GetEntries() const {
    return log_entries_;
}

const std::vector<LogEntry>& LogProcessor::GetFilteredEntries() const {
    return filtered_entries_;
}

size_t LogProcessor::GetTotalEntryCount() const {
    return log_entries_.size();
}

size_t LogProcessor::GetFilteredEntryCount() const {
    return filtered_entries_.size();
}

void LogProcessor::SetCurrentIndex(int index) {
    current_index_ = ClampIndex(index);
}

int LogProcessor::GetCurrentIndex() const {
    return current_index_;
}

bool LogProcessor::CanNavigateUp() const {
    return current_index_ > 0;
}

bool LogProcessor::CanNavigateDown() const {
    const auto& active_entries = GetActiveEntries();
    return current_index_ < static_cast<int>(active_entries.size()) - 1;
}

void LogProcessor::NavigateUp(int count) {
    SetCurrentIndex(current_index_ - count);
}

void LogProcessor::NavigateDown(int count) {
    SetCurrentIndex(current_index_ + count);
}

void LogProcessor::NavigateToTop() {
    SetCurrentIndex(0);
}

void LogProcessor::NavigateToBottom() {
    const auto& active_entries = GetActiveEntries();
    if (!active_entries.empty()) {
        SetCurrentIndex(static_cast<int>(active_entries.size()) - 1);
    }
}

void LogProcessor::NavigateToPercentage(int percentage) {
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    const auto& active_entries = GetActiveEntries();
    if (active_entries.empty()) return;
    
    int target_index = (static_cast<int>(active_entries.size()) - 1) * percentage / 100;
    SetCurrentIndex(target_index);
}

int LogProcessor::GetCurrentPercentage() const {
    const auto& active_entries = GetActiveEntries();
    if (active_entries.empty()) return 0;
    
    return (current_index_ * 100) / static_cast<int>(active_entries.size() - 1);
}

std::vector<int> LogProcessor::Search(const std::string& query, bool case_sensitive) const {
    std::vector<int> results;
    if (query.empty()) return results;
    
    const auto& active_entries = GetActiveEntries();
    
    try {
        std::regex search_regex;
        if (case_sensitive) {
            search_regex = std::regex(query);
        } else {
            search_regex = std::regex(query, std::regex_constants::icase);
        }
        
        for (size_t i = 0; i < active_entries.size(); ++i) {
            const auto& entry = active_entries[i];
            
            // Search in message, logger, and log level
            bool found = std::regex_search(entry.Get_message(), search_regex) ||
                        std::regex_search(entry.Get_logger_name(), search_regex);
            
            // Check log level if it exists
            if (!found && entry.Get_log_level().has_value()) {
                found = std::regex_search(entry.Get_log_level().value(), search_regex);
            }
            
            if (found) {
                results.push_back(static_cast<int>(i));
            }
        }
    } catch (const std::regex_error&) {
        // If regex fails, fall back to simple string search
        std::string search_query = query;
        if (!case_sensitive) {
            std::transform(search_query.begin(), search_query.end(), search_query.begin(), ::tolower);
        }
        
        for (size_t i = 0; i < active_entries.size(); ++i) {
            const auto& entry = active_entries[i];
            
            std::string message = entry.Get_message();
            std::string logger = entry.Get_logger_name();
            std::string log_level = entry.Get_log_level().has_value() ? entry.Get_log_level().value() : "";
            
            if (!case_sensitive) {
                std::transform(message.begin(), message.end(), message.begin(), ::tolower);
                std::transform(logger.begin(), logger.end(), logger.begin(), ::tolower);
                std::transform(log_level.begin(), log_level.end(), log_level.begin(), ::tolower);
            }
            
            if (message.find(search_query) != std::string::npos ||
                logger.find(search_query) != std::string::npos ||
                (!log_level.empty() && log_level.find(search_query) != std::string::npos)) {
                results.push_back(static_cast<int>(i));
            }
        }
    }
    
    return results;
}

void LogProcessor::SetSearchResults(const std::vector<int>& results) {
    search_results_ = results;
    current_search_index_ = results.empty() ? -1 : 0;
}

const std::vector<int>& LogProcessor::GetSearchResults() const {
    return search_results_;
}

int LogProcessor::GetCurrentSearchIndex() const {
    return current_search_index_;
}

void LogProcessor::SetCurrentSearchIndex(int index) {
    if (search_results_.empty()) {
        current_search_index_ = -1;
    } else {
        current_search_index_ = std::clamp(index, 0, static_cast<int>(search_results_.size()) - 1);
    }
}

bool LogProcessor::HasSearchResults() const {
    return !search_results_.empty();
}

void LogProcessor::NavigateToNextSearchResult() {
    if (search_results_.empty()) return;
    
    current_search_index_ = (current_search_index_ + 1) % static_cast<int>(search_results_.size());
    SetCurrentIndex(search_results_[current_search_index_]);
}

void LogProcessor::NavigateToPreviousSearchResult() {
    if (search_results_.empty()) return;
    
    current_search_index_--;
    if (current_search_index_ < 0) {
        current_search_index_ = static_cast<int>(search_results_.size()) - 1;
    }
    SetCurrentIndex(search_results_[current_search_index_]);
}

void LogProcessor::ClearSearchResults() {
    search_results_.clear();
    current_search_index_ = -1;
}

void LogProcessor::SetFilteredEntries(const std::vector<LogEntry>& filtered_entries) {
    filtered_entries_ = filtered_entries;
    
    // Reset current index if it's out of bounds
    if (current_index_ >= static_cast<int>(filtered_entries_.size())) {
        current_index_ = std::max(0, static_cast<int>(filtered_entries_.size()) - 1);
    }
    
    // Clear search results as they may no longer be valid
    ClearSearchResults();
}

void LogProcessor::ResetToAllEntries() {
    filtered_entries_ = log_entries_;
    
    // Reset current index if it's out of bounds
    if (current_index_ >= static_cast<int>(filtered_entries_.size())) {
        current_index_ = std::max(0, static_cast<int>(filtered_entries_.size()) - 1);
    }
    
    // Clear search results as they may no longer be valid
    ClearSearchResults();
}

bool LogProcessor::IsValidIndex(int index) const {
    const auto& active_entries = GetActiveEntries();
    return index >= 0 && index < static_cast<int>(active_entries.size());
}

int LogProcessor::ClampIndex(int index) const {
    const auto& active_entries = GetActiveEntries();
    if (active_entries.empty()) return 0;
    
    return std::clamp(index, 0, static_cast<int>(active_entries.size()) - 1);
}

void LogProcessor::UpdateCurrentIndex() {
    current_index_ = ClampIndex(current_index_);
}

const std::vector<LogEntry>& LogProcessor::GetActiveEntries() const {
    return filtered_entries_.empty() ? log_entries_ : filtered_entries_;
}

} // namespace ue_log
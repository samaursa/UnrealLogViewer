#pragma once

#include "../log_parser/log_entry.h"
#include "../log_parser/log_parser.h"
#include <memory>
#include <vector>
#include <string>

namespace ue_log {

/**
 * Core business logic for log processing without UI dependencies.
 * Handles file loading, entry management, navigation, and search functionality
 * in a headless manner suitable for automated testing.
 */
class LogProcessor {
public:
    LogProcessor();
    ~LogProcessor();
    
    // File operations
    bool LoadFile(const std::string& file_path);
    void ClearEntries();
    bool IsFileLoaded() const;
    std::string GetCurrentFilePath() const;
    std::string GetLastError() const;
    
    // Entry access
    const std::vector<LogEntry>& GetEntries() const;
    const std::vector<LogEntry>& GetFilteredEntries() const;
    size_t GetTotalEntryCount() const;
    size_t GetFilteredEntryCount() const;
    
    // Navigation without UI
    void SetCurrentIndex(int index);
    int GetCurrentIndex() const;
    bool CanNavigateUp() const;
    bool CanNavigateDown() const;
    void NavigateUp(int count = 1);
    void NavigateDown(int count = 1);
    void NavigateToTop();
    void NavigateToBottom();
    void NavigateToPercentage(int percentage);
    int GetCurrentPercentage() const;
    
    // Search functionality
    std::vector<int> Search(const std::string& query, bool case_sensitive = false) const;
    void SetSearchResults(const std::vector<int>& results);
    const std::vector<int>& GetSearchResults() const;
    int GetCurrentSearchIndex() const;
    void SetCurrentSearchIndex(int index);
    bool HasSearchResults() const;
    void NavigateToNextSearchResult();
    void NavigateToPreviousSearchResult();
    void ClearSearchResults();
    
    // Filter integration
    void SetFilteredEntries(const std::vector<LogEntry>& filtered_entries);
    void ResetToAllEntries();
    
    // Validation and bounds checking
    bool IsValidIndex(int index) const;
    int ClampIndex(int index) const;
    
private:
    std::unique_ptr<LogParser> log_parser_;
    std::vector<LogEntry> log_entries_;
    std::vector<LogEntry> filtered_entries_;
    std::string current_file_path_;
    std::string last_error_;
    
    // Navigation state
    int current_index_;
    
    // Search state
    std::vector<int> search_results_;
    int current_search_index_;
    
    // Helper methods
    void UpdateCurrentIndex();
    const std::vector<LogEntry>& GetActiveEntries() const;
};

} // namespace ue_log
#pragma once
#include <vector>
#include <atomic>
#include <string>
#include <fstream>

#include "ftxui/component/component.hpp"
#include "log_entry.h"
#include "filter_manager.h"

class LogViewer {
private:
    std::vector<LogEntry> log_entries_;
    std::vector<size_t> filtered_indices_;
    FilterManager filter_manager_;
    UnrealLogParser parser_;
    std::ifstream tail_file_;
    std::streampos last_file_pos_;

    // UI State
    std::string file_path_;
    std::string search_term_;
    std::string error_message_;
    bool tailing_enabled_ = false;
    bool show_filters_ = true;
    bool show_expanded_ = true;
    int selected_line_ = 0;
    int scroll_offset_ = 0;
    int expanded_height_ = 6;

    // Mode switching
    enum class InputMode { SELECTION, INPUT };
    InputMode current_mode_ = InputMode::SELECTION;
    int selected_window_ = 2; // Default to log view

    // Performance optimization
    bool needs_filter_update_ = true;
    std::string last_search_term_;
    bool escape_pressed_ = false;

public:
    ~LogViewer();

    void UpdateFilteredEntries();
    void LoadFile();
    void StartTailing();
    void CheckForNewLines();
    ftxui::Component CreateUI();
    void Run();

    // Getters for UI
    const std::vector<LogEntry>& GetLogEntries() const { return log_entries_; }
    const std::vector<size_t>& GetFilteredIndices() const { return filtered_indices_; }
    FilterManager& GetFilterManager() { return filter_manager_; }

    // UI State getters/setters
    std::string& GetFilePath() { return file_path_; }
    void SetFilePath(const std::string& path) { file_path_ = path; }

    std::string& GetSearchTerm() { return search_term_; }
    void SetSearchTerm(const std::string& term) { search_term_ = term; needs_filter_update_ = true; }

    const std::string& GetErrorMessage() const { return error_message_; }

    bool IsTailingEnabled() const { return tailing_enabled_; }
    void SetTailingEnabled(bool enabled) { tailing_enabled_ = enabled; }

    bool ShowFilters() const { return show_filters_; }
    void SetShowFilters(bool show) { show_filters_ = show; }

    bool ShowExpanded() const { return show_expanded_; }
    void SetShowExpanded(bool show) { show_expanded_ = show; }

    int GetSelectedLine() const { return selected_line_; }
    void SetSelectedLine(int line) { selected_line_ = line; }

    int GetScrollOffset() const { return scroll_offset_; }
    void SetScrollOffset(int offset) { scroll_offset_ = offset; }

    int GetExpandedHeight() const { return expanded_height_; }
};

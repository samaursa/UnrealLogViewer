#include "log_viewer.h"
#include <algorithm>
#include <chrono>
#include <fstream>

LogViewer::~LogViewer() {
    if (tail_file_.is_open()) {
        tail_file_.close();
    }
}

void LogViewer::UpdateFilteredEntries() {
    if (!needs_filter_update_ && last_search_term_ == search_term_) {
        return;
    }

    // Safety check - don't update if vector is being used elsewhere
    static bool updating = false;
    if (updating) return;
    updating = true;

    filtered_indices_.clear();
    filtered_indices_.reserve(log_entries_.size());

    for (size_t i = 0; i < log_entries_.size(); ++i) {
        if (filter_manager_.MatchesFilters(log_entries_[i], search_term_)) {
            filtered_indices_.push_back(i);
        }
    }

    needs_filter_update_ = false;
    last_search_term_ = search_term_;
    updating = false;
}

void LogViewer::LoadFile() {
    if (tail_file_.is_open()) {
        tail_file_.close();
    }

    std::ifstream file(file_path_);
    if (!file.is_open()) {
        error_message_ = "Could not open file: " + file_path_;
        return;
    }

    error_message_.clear();
    filter_manager_.GetCategories().clear();

    log_entries_.clear();
    log_entries_.reserve(100000);

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        auto entry = parser_.ParseLine(line);
        log_entries_.push_back(entry);

        if (!entry.category.empty()) {
            filter_manager_.AddCategory(entry.category);
        }
    }

    needs_filter_update_ = true;
    selected_line_ = 0;
    scroll_offset_ = 0;

    if (tailing_enabled_) {
        StartTailing();
    }
}

void LogViewer::StartTailing() {
    tail_file_.open(file_path_, std::ios::ate);
    if (tail_file_.is_open()) {
        last_file_pos_ = tail_file_.tellg();
    }
}

void LogViewer::CheckForNewLines() {
    if (!tailing_enabled_ || !tail_file_.is_open()) {
        return;
    }

    tail_file_.clear();
    tail_file_.seekg(0, std::ios::end);
    auto current_pos = tail_file_.tellg();

    if (current_pos > last_file_pos_) {
        tail_file_.seekg(last_file_pos_);
        std::string line;

        while (std::getline(tail_file_, line)) {
            if (!line.empty()) {
                auto entry = parser_.ParseLine(line);
                log_entries_.push_back(entry);

                if (!entry.category.empty()) {
                    filter_manager_.AddCategory(entry.category);
                }
            }
        }

        needs_filter_update_ = true;
        last_file_pos_ = current_pos;
    }
}

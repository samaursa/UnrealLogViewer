#include "log_window.h"
#include <algorithm>

using namespace ftxui;

LogWindow::LogWindow(int id, const std::string& title)
    : id_(id), title_(title), selected_line_(0), scroll_offset_(0), log_entries_(nullptr) {}

void LogWindow::SetLogEntries(const std::vector<LogEntry>* entries) {
    log_entries_ = entries;
    selected_line_ = 0;
    scroll_offset_ = 0;
}

bool LogWindow::HandleEvent(Event event) {
    int total_logs = log_entries_ ? log_entries_->size() : 0;
    if (total_logs == 0) return false;

    if (event == Event::ArrowUp && selected_line_ > 0) {
        selected_line_--;
        if (selected_line_ < scroll_offset_) {
            scroll_offset_ = selected_line_;
        }
        return true;
    }

    if (event == Event::ArrowDown && selected_line_ < total_logs - 1) {
        selected_line_++;
        const int visible_lines = 20;
        if (selected_line_ >= scroll_offset_ + visible_lines) {
            scroll_offset_ = selected_line_ - visible_lines + 1;
        }
        return true;
    }

    if (event == Event::PageUp) {
        selected_line_ = std::max(0, selected_line_ - 10);
        if (selected_line_ < scroll_offset_) {
            scroll_offset_ = std::max(0, selected_line_);
        }
        return true;
    }

    if (event == Event::PageDown) {
        selected_line_ = std::min(total_logs - 1, selected_line_ + 10);
        const int visible_lines = 20;
        if (selected_line_ >= scroll_offset_ + visible_lines) {
            scroll_offset_ = std::min(total_logs - visible_lines, selected_line_ - visible_lines + 1);
        }
        return true;
    }

    return false;
}

const LogEntry* LogWindow::GetSelectedEntry() const {
    if (!log_entries_ || selected_line_ >= log_entries_->size()) {
        return nullptr;
    }
    return &(*log_entries_)[selected_line_];
}

Element LogWindow::Render(bool is_selected, int available_height) const {
    std::string display_title = title_;
    if (!is_selected) {
        std::transform(display_title.begin(), display_title.end(), display_title.begin(), ::tolower);
    }

    auto title_text = "[" + std::to_string(id_) + "] " + display_title;

    Elements log_lines;
    log_lines.push_back(text("Time | Category | Level | Message") | bold);

    int visible_lines = std::max(5, available_height - 3);

    if (!log_entries_ || log_entries_->empty()) {
        log_lines.push_back(text("No log entries loaded"));
    } else {
        for (int i = 0; i < visible_lines; ++i) {
            int line_idx = scroll_offset_ + i;
            if (line_idx >= log_entries_->size()) break;

            const auto& entry = (*log_entries_)[line_idx];
            std::string line = entry.timestamp + " | " + entry.category + " | " + entry.level + " | " + entry.message;
            if (line.length() > 120) line = line.substr(0, 117) + "...";

            auto element = text(line);
            if (line_idx == selected_line_) {
                element = element | inverted;
            }
            log_lines.push_back(element);
        }
    }

    int total = log_entries_ ? log_entries_->size() : 0;
    // Removed status line - will be shown in main status bar

    return window(text(title_text), vbox(log_lines));
}

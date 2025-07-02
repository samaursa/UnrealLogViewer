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

    // Go to top
    if (event == Event::Character('<')) {
        selected_line_ = 0;
        scroll_offset_ = 0;
        return true;
    }

    // Go to bottom
    if (event == Event::Character('>')) {
        selected_line_ = total_logs - 1;
        const int visible_lines = 20;
        scroll_offset_ = std::max(0, total_logs - visible_lines);
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

    // Table header
    log_lines.push_back(
        hbox({
            text("Time") | bold | size(WIDTH, EQUAL, 12),
            text("│"),
            text("Frame") | bold | size(WIDTH, EQUAL, 6),
            text("│"),
            text("Category") | bold | size(WIDTH, EQUAL, 20),
            text("│"),
            text("Level") | bold | size(WIDTH, EQUAL, 10),
            text("│"),
            text("Message") | bold | flex,
        }) | color(Color::Yellow)
    );

    // Separator line
    log_lines.push_back(
        hbox({
            text(std::string(12, '─')) | size(WIDTH, EQUAL, 12),
            text("┼"),
            text(std::string(6, '─')) | size(WIDTH, EQUAL, 6),
            text("┼"),
            text(std::string(20, '─')) | size(WIDTH, EQUAL, 20),
            text("┼"),
            text(std::string(10, '─')) | size(WIDTH, EQUAL, 10),
            text("┼"),
            text("─") | flex,
        }) | color(Color::GrayLight)
    );

    int visible_lines = std::max(5, available_height - 5); // Account for header + separator

    if (!log_entries_ || log_entries_->empty()) {
        log_lines.push_back(text("No log entries loaded"));
    } else {
        for (int i = 0; i < visible_lines; ++i) {
            int line_idx = scroll_offset_ + i;
            if (line_idx >= log_entries_->size()) break;

            const auto& entry = (*log_entries_)[line_idx];

            // Truncate fields to fit columns
            std::string time_str = entry.timestamp.length() > 12 ?
                entry.timestamp.substr(0, 12) : entry.timestamp;
            std::string frame_str = entry.frame.length() > 6 ?
                entry.frame.substr(0, 6) : entry.frame;
            std::string cat_str = entry.category.length() > 20 ?
                entry.category.substr(0, 20) : entry.category;
            std::string level_str = entry.level.length() > 10 ?
                entry.level.substr(0, 10) : entry.level;

            auto row = hbox({
                text(time_str) | size(WIDTH, EQUAL, 12),
                text("│"),
                text(frame_str) | size(WIDTH, EQUAL, 6),
                text("│"),
                text(cat_str) | size(WIDTH, EQUAL, 20),
                text("│"),
                text(level_str) | size(WIDTH, EQUAL, 10),
                text("│"),
                text(entry.message) | flex,
            });

            if (line_idx == selected_line_) {
                row = row | inverted;
            }

            log_lines.push_back(row);
        }
    }

    int total = log_entries_ ? log_entries_->size() : 0;

    auto table_content = vbox(log_lines);

    // Add custom scrollbar if needed
    if (total > visible_lines) {
        // Calculate scrollbar position
        float scroll_ratio = (float)scroll_offset_ / (total - visible_lines);
        int scrollbar_height = visible_lines;
        int thumb_pos = (int)(scroll_ratio * (scrollbar_height - 1));

        Elements scrollbar_elements;
        for (int i = 0; i < scrollbar_height; ++i) {
            if (i == thumb_pos) {
                scrollbar_elements.push_back(text("█"));
            } else {
                scrollbar_elements.push_back(text("│"));
            }
        }

        return window(text(title_text), hbox({
            table_content | flex,
            vbox(scrollbar_elements) | size(WIDTH, EQUAL, 1)
        }));
    }

    return window(text(title_text), table_content);
}

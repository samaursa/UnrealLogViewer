#include "log_window.h"
#include <algorithm>

using namespace ftxui;

LogWindow::LogWindow(int id, const std::string& title)
    : id_(id), title_(title), selected_line_(0), scroll_offset_(0) {}

bool LogWindow::HandleEvent(Event event) {
    // Mock log count for testing
    const int total_logs = 100;

    if (event == Event::ArrowUp && selected_line_ > 0) {
        selected_line_--;
        if (selected_line_ < scroll_offset_) {
            scroll_offset_ = selected_line_;
        }
        return true;
    }

    if (event == Event::ArrowDown && selected_line_ < total_logs - 1) {
        selected_line_++;
        const int visible_lines = 20; // Mock visible area
        if (selected_line_ >= scroll_offset_ + visible_lines) {
            scroll_offset_ = selected_line_ - visible_lines + 1;
        }
        return true;
    }

    if (event == Event::PageUp) {
        selected_line_ = std::max(0, selected_line_ - 10);
        scroll_offset_ = std::max(0, scroll_offset_ - 10);
        return true;
    }

    if (event == Event::PageDown) {
        selected_line_ = std::min(total_logs - 1, selected_line_ + 10);
        scroll_offset_ = std::min(total_logs - 20, scroll_offset_ + 10);
        return true;
    }

    return false;
}

Element LogWindow::Render(bool is_selected) const {
    std::string display_title = title_;
    if (!is_selected) {
        std::transform(display_title.begin(), display_title.end(), display_title.begin(), ::tolower);
    }

    auto title_text = "[" + std::to_string(id_) + "] " + display_title;

    Elements log_lines;
    log_lines.push_back(text("Time | Category | Level | Message") | bold);

    // Mock log entries for testing
    for (int i = 0; i < 15; ++i) {
        int line_num = scroll_offset_ + i;
        std::string line = "Log entry " + std::to_string(line_num);

        auto element = text(line);
        if (line_num == selected_line_) {
            element = element | inverted;
        }
        log_lines.push_back(element);
    }

    log_lines.push_back(text("Selected: " + std::to_string(selected_line_) +
                             " | Scroll: " + std::to_string(scroll_offset_)));

    return window(text(title_text), vbox(log_lines));
}
#include "expanded_window.h"
#include <algorithm>

using namespace ftxui;

ExpandedWindow::ExpandedWindow(int id, const std::string& title)
    : id_(id), title_(title) {}

Element ExpandedWindow::Render(bool is_selected, int selected_log_line) const {
    std::string display_title = title_;
    if (!is_selected) {
        std::transform(display_title.begin(), display_title.end(), display_title.begin(), ::tolower);
    }

    auto title_text = "[" + std::to_string(id_) + "] " + display_title;

    return window(text(title_text), vbox({
        text("Selected log line details:") | bold,
        text("Line: " + std::to_string(selected_log_line)),
        text("Raw: Mock log entry " + std::to_string(selected_log_line)),
        text("Category: LogTemp"),
        text("Level: Display"),
        text("Time: 12:34:56"),
        text("Message: This is a mock message for line " + std::to_string(selected_log_line))
    }));
}
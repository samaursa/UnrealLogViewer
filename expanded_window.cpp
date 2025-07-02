#include "expanded_window.h"
#include <algorithm>

using namespace ftxui;

ExpandedWindow::ExpandedWindow(int id, const std::string& title)
    : id_(id), title_(title) {}

Element ExpandedWindow::Render(bool is_selected, const LogEntry* selected_entry) const {
    std::string display_title = title_;
    if (!is_selected) {
        std::transform(display_title.begin(), display_title.end(), display_title.begin(), ::tolower);
    }

    auto title_text = "[" + std::to_string(id_) + "] " + display_title;

    if (!selected_entry) {
        return window(text(title_text), vbox({
            text("No log entry selected")
        }));
    }

    return window(text(title_text), vbox({
        text("Selected log line details:") | bold,
        text("Raw: " + selected_entry->raw_line),
        text("Time: " + selected_entry->timestamp),
        text("Category: " + selected_entry->category),
        text("Level: " + selected_entry->level),
        text("Message: " + selected_entry->message)
    }));
}

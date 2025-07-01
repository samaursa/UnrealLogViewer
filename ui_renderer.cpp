#include "ui_renderer.h"
#include "log_viewer.h"
#include "ftxui/screen/terminal.hpp"
#include <algorithm>

using namespace ftxui;

Component UIRenderer::CreateEventHandler(Component main_container) {
    return main_container |= CatchEvent([this](Event event) -> bool {
        int max_lines = static_cast<int>(viewer_->GetFilteredIndices().size());
        if (max_lines == 0) return false;

        // Mouse events
        if (event.is_mouse()) {
            if (event.mouse().button == Mouse::WheelUp) {
                int new_line = std::max(0, viewer_->GetSelectedLine() - 3);
                viewer_->SetSelectedLine(new_line);
                if (new_line < viewer_->GetScrollOffset()) {
                    viewer_->SetScrollOffset(new_line);
                }
                return true;
            }
            if (event.mouse().button == Mouse::WheelDown) {
                int new_line = std::min(max_lines - 1, viewer_->GetSelectedLine() + 3);
                viewer_->SetSelectedLine(new_line);
                int visible_lines = viewer_->ShowExpanded() ? 20 : 25;
                if (new_line >= viewer_->GetScrollOffset() + visible_lines) {
                    viewer_->SetScrollOffset(new_line - visible_lines + 1);
                }
                return true;
            }
            return false;
        }

        // Keyboard events
        if (event == Event::ArrowUp && viewer_->GetSelectedLine() > 0) {
            int new_line = viewer_->GetSelectedLine() - 1;
            viewer_->SetSelectedLine(new_line);
            if (new_line < viewer_->GetScrollOffset()) {
                viewer_->SetScrollOffset(new_line);
            }
            return true;
        }
        if (event == Event::ArrowDown && viewer_->GetSelectedLine() < max_lines - 1) {
            int new_line = viewer_->GetSelectedLine() + 1;
            viewer_->SetSelectedLine(new_line);
            int visible_lines = viewer_->ShowExpanded() ? 20 : 25;
            if (new_line >= viewer_->GetScrollOffset() + visible_lines) {
                viewer_->SetScrollOffset(new_line - visible_lines + 1);
            }
            return true;
        }

        // Toggle expanded view
        if (event == Event::Character('e') || event == Event::Character('E')) {
            viewer_->SetShowExpanded(!viewer_->ShowExpanded());
            return true;
        }

        // Category toggles
        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= '1' && c <= '9') {
                int filter_index = c - '1';
                auto& categories = viewer_->GetFilterManager().GetCategories();
                auto it = categories.begin();
                std::advance(it, filter_index);
                if (it != categories.end()) {
                    viewer_->GetFilterManager().ToggleCategory(it->first);
                    viewer_->SetSearchTerm(viewer_->GetSearchTerm()); // Trigger update
                    return true;
                }
            }
        }

        return false;
    });
}

Element UIRenderer::RenderHeader(Component file_input, Component load_button,
                                Component tail_checkbox, Component filter_checkbox,
                                Component search_input) const {
    return vbox({
        hbox({
            text("File: ") | size(WIDTH, EQUAL, 6),
            file_input->Render() | flex,
            text(" "),
            load_button->Render(),
            text(" "),
            tail_checkbox->Render(),
            text(" "),
            filter_checkbox->Render(),
        }),
        hbox({
            text("Search: ") | size(WIDTH, EQUAL, 8),
            search_input->Render() | flex,
            text(" Lines: " + std::to_string(viewer_->GetFilteredIndices().size()) +
                 "/" + std::to_string(viewer_->GetLogEntries().size())),
        }),
    });
}

Element UIRenderer::RenderLogTable(int table_height) const {
    Elements log_elements;
    log_elements.push_back(
        hbox({
            text("Time") | bold | size(WIDTH, EQUAL, 12),
            text("│"),
            text("Category") | bold | size(WIDTH, EQUAL, 20),
            text("│"),
            text("Level") | bold | size(WIDTH, EQUAL, 10),
            text("│"),
            text("Message") | bold | flex,
        }) | color(Color::Yellow)
    );

    int visible_start = viewer_->GetScrollOffset();
    int visible_end = std::min(visible_start + table_height,
                               static_cast<int>(viewer_->GetFilteredIndices().size()));

    for (int i = visible_start; i < visible_end; ++i) {
        if (i >= static_cast<int>(viewer_->GetFilteredIndices().size())) break;

        size_t entry_idx = viewer_->GetFilteredIndices()[i];
        const auto& entry = viewer_->GetLogEntries()[entry_idx];
        Color entry_color = viewer_->GetFilterManager().GetEntryColor(entry);

        std::string time_str = entry.timestamp;
        if (time_str.length() > 12) {
            size_t time_start = time_str.find('-');
            if (time_start != std::string::npos && time_start + 1 < time_str.length()) {
                time_str = time_str.substr(time_start + 1);
                if (time_str.length() > 12) time_str = time_str.substr(0, 12);
            }
        }

        std::string cat_str = entry.category;
        if (cat_str.length() > 19) cat_str = cat_str.substr(0, 19);

        std::string level_str = entry.level;
        if (level_str.length() > 9) level_str = level_str.substr(0, 9);

        auto row = hbox({
            text(time_str) | size(WIDTH, EQUAL, 12),
            text("│"),
            text(cat_str) | size(WIDTH, EQUAL, 20),
            text("│"),
            text(level_str) | size(WIDTH, EQUAL, 10),
            text("│"),
            text(entry.message) | flex,
        }) | color(entry_color);

        if (i == viewer_->GetSelectedLine()) {
            row = row | inverted;
        }

        log_elements.push_back(row);
    }

    return vbox(log_elements) | size(HEIGHT, EQUAL, table_height + 1) | border;
}

Element UIRenderer::RenderExpandedView() const {
    Elements expanded_elements;
    if (viewer_->ShowExpanded() && !viewer_->GetFilteredIndices().empty() &&
        viewer_->GetSelectedLine() >= 0 &&
        viewer_->GetSelectedLine() < static_cast<int>(viewer_->GetFilteredIndices().size())) {

        size_t entry_idx = viewer_->GetFilteredIndices()[viewer_->GetSelectedLine()];
        const auto& entry = viewer_->GetLogEntries()[entry_idx];

        expanded_elements.push_back(text("Expanded View (Press 'E' to toggle):") | bold | color(Color::Yellow));
        expanded_elements.push_back(text("Raw: " + entry.raw_line) | color(Color::White));

        if (!entry.timestamp.empty()) {
            expanded_elements.push_back(text("Time: " + entry.timestamp) | color(Color::Cyan));
        }
        if (!entry.category.empty()) {
            expanded_elements.push_back(text("Category: " + entry.category) |
                                      color(viewer_->GetFilterManager().GetEntryColor(entry)));
        }
        if (!entry.level.empty()) {
            expanded_elements.push_back(text("Level: " + entry.level) | color(Color::Green));
        }
    }

    return vbox(expanded_elements) | border | size(HEIGHT, EQUAL, viewer_->GetExpandedHeight());
}

Element UIRenderer::RenderFilterPanel(int filter_width, int log_height) const {
    Elements filter_elements;
    if (viewer_->ShowFilters() && !viewer_->GetFilterManager().GetCategories().empty()) {
        filter_elements.push_back(text("Categories (1-9):") | bold | color(Color::Yellow));
        filter_elements.push_back(text("─────────────") | color(Color::GrayLight));

        int idx = 0;
        for (const auto& [name, filter] : viewer_->GetFilterManager().GetCategories()) {
            std::string prefix = (idx < 9) ? std::to_string(idx + 1) + ": " : "  ";

            filter_elements.push_back(
                hbox({
                    text(prefix) | color(Color::GrayLight) | size(WIDTH, EQUAL, 3),
                    text(filter.enabled ? "[x] " : "[ ] ") |
                        (filter.enabled ? color(Color::Green) : color(Color::Red)) | size(WIDTH, EQUAL, 4),
                    text(name) | color(filter.color) | flex,
                })
            );
            idx++;
        }
    }

    return vbox(filter_elements) | border | size(WIDTH, EQUAL, filter_width) | size(HEIGHT, EQUAL, log_height);
}

Element UIRenderer::RenderStatusBar() const {
    return hbox({
        text("Line: " + std::to_string(viewer_->GetSelectedLine() + 1)),
        text(" | "),
        text(viewer_->IsTailingEnabled() ? "TAILING" : "STATIC") |
            (viewer_->IsTailingEnabled() ? color(Color::Green) : color(Color::Red)),
        text(" | "),
        text("Cats: " + std::to_string(viewer_->GetFilterManager().GetCategories().size())),
        text(" | "),
        text("↑↓: Navigate, E: Expand, 1-9: Toggle"),
    }) | border;
}

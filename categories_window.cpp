#include "categories_window.h"
#include <algorithm>

using namespace ftxui;

CategoriesWindow::CategoriesWindow(int id, const std::string& title)
    : id_(id), title_(title), filter_manager_(nullptr), selected_category_(0), scroll_offset_(0) {}

void CategoriesWindow::SetFilterManager(FilterManager* manager) {
    filter_manager_ = manager;
    selected_category_ = 0;
    scroll_offset_ = 0;
}

bool CategoriesWindow::HandleEvent(Event event) {
    if (!filter_manager_) return false;

    auto& categories = filter_manager_->GetCategories();
    int total_categories = categories.size();
    if (total_categories == 0) return false;

    if (event == Event::ArrowUp && selected_category_ > 0) {
        selected_category_--;
        if (selected_category_ < scroll_offset_) {
            scroll_offset_ = selected_category_;
        }
        return true;
    }

    if (event == Event::ArrowDown && selected_category_ < total_categories - 1) {
        selected_category_++;
        // Use dynamic visible lines based on available height
        const int visible_lines = 20; // Will be updated by available height
        if (selected_category_ >= scroll_offset_ + visible_lines) {
            scroll_offset_ = selected_category_ - visible_lines + 1;
        }
        return true;
    }

    if (event == Event::Character(' ')) {
        auto it = categories.begin();
        std::advance(it, selected_category_);
        if (it != categories.end()) {
            filter_manager_->ToggleCategory(it->first);
            return true;
        }
    }

    // Shift+Space alternative: Use 'S' key for isolate
    if (event == Event::Character('S')) {
        auto selected_it = categories.begin();
        std::advance(selected_it, selected_category_);
        if (selected_it != categories.end()) {
            bool any_others_enabled = false;
            for (const auto& [name, filter] : categories) {
                if (name != selected_it->first && filter.enabled) {
                    any_others_enabled = true;
                    break;
                }
            }

            if (any_others_enabled) {
                // Turn off all except selected
                for (const auto& [name, filter] : categories) {
                    if (name != selected_it->first && filter.enabled) {
                        filter_manager_->ToggleCategory(name);
                    }
                }
            } else {
                // Turn all back on
                for (const auto& [name, filter] : categories) {
                    if (!filter.enabled) {
                        filter_manager_->ToggleCategory(name);
                    }
                }
            }
            return true;
        }
    }

    return false;
}

Element CategoriesWindow::Render(bool is_selected, int available_height) const {
    std::string display_title = title_;
    if (!is_selected) {
        std::transform(display_title.begin(), display_title.end(), display_title.begin(), ::tolower);
    }

    auto title_text = "[" + std::to_string(id_) + "] " + display_title;

    Elements category_elements;
    category_elements.push_back(text("Categories (Space:toggle, S:isolate):") | bold | color(Color::Yellow));
    category_elements.push_back(text("─────────────────────────────────────") | color(Color::GrayLight));

    if (!filter_manager_ || filter_manager_->GetCategories().empty()) {
        category_elements.push_back(text("No categories loaded"));
    } else {
        const int visible_lines = std::max(5, available_height - 3); // Account for header
        int idx = 0;
        for (const auto& [name, filter] : filter_manager_->GetCategories()) {
            if (idx < scroll_offset_) {
                idx++;
                continue;
            }
            if (idx >= scroll_offset_ + visible_lines) break;

            std::string checkbox = filter.enabled ? "[x] " : "[ ] ";

            auto element = hbox({
                text(checkbox) | (filter.enabled ? color(Color::Green) : color(Color::Red)) | size(WIDTH, EQUAL, 4),
                text(name) | color(filter.color) | flex,
            });

            if (is_selected && idx == selected_category_) {
                element = element | inverted;
            }

            category_elements.push_back(element);
            idx++;
        }
    }

    return window(text(title_text), vbox(category_elements));
}

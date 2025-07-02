#include "categories_window.h"
#include <algorithm>

using namespace ftxui;

CategoriesWindow::CategoriesWindow(int id, const std::string& title)
    : id_(id), title_(title), filter_manager_(nullptr) {}

void CategoriesWindow::SetFilterManager(FilterManager* manager) {
    filter_manager_ = manager;
}

bool CategoriesWindow::HandleEvent(Event event) {
    if (!filter_manager_) return false;

    if (event.is_character()) {
        char c = event.character()[0];
        if (c >= '1' && c <= '9') {
            int category_idx = c - '1';
            auto& categories = filter_manager_->GetCategories();
            auto it = categories.begin();
            std::advance(it, category_idx);
            if (it != categories.end()) {
                filter_manager_->ToggleCategory(it->first);
                return true;
            }
        }
    }

    return false;
}

Element CategoriesWindow::Render(bool is_selected) const {
    std::string display_title = title_;
    if (!is_selected) {
        std::transform(display_title.begin(), display_title.end(), display_title.begin(), ::tolower);
    }

    auto title_text = "[" + std::to_string(id_) + "] " + display_title;

    Elements category_elements;
    category_elements.push_back(text("Categories (1-9):") | bold | color(Color::Yellow));
    category_elements.push_back(text("─────────────") | color(Color::GrayLight));

    if (!filter_manager_ || filter_manager_->GetCategories().empty()) {
        category_elements.push_back(text("No categories loaded"));
    } else {
        int idx = 0;
        for (const auto& [name, filter] : filter_manager_->GetCategories()) {
            if (idx >= 9) break; // Only show first 9

            std::string prefix = std::to_string(idx + 1) + ": ";
            std::string checkbox = filter.enabled ? "[x] " : "[ ] ";

            auto element = hbox({
                text(prefix) | color(Color::GrayLight) | size(WIDTH, EQUAL, 3),
                text(checkbox) | (filter.enabled ? color(Color::Green) : color(Color::Red)) | size(WIDTH, EQUAL, 4),
                text(name) | color(filter.color) | flex,
            });

            category_elements.push_back(element);
            idx++;
        }
    }

    return window(text(title_text), vbox(category_elements));
}
#pragma once
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/event.hpp"
#include "filter_manager.h"

class CategoriesWindow {
private:
    int id_;
    std::string title_;
    FilterManager* filter_manager_;
    int selected_category_;
    int scroll_offset_;

public:
    CategoriesWindow(int id, const std::string& title);

    void SetFilterManager(FilterManager* manager);
    bool HandleEvent(ftxui::Event event);
    ftxui::Element Render(bool is_selected, int available_height = 15) const;
};

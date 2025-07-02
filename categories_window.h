#pragma once
#include "filter_manager.h"
#include "ftxui/component/event.hpp"
#include "ftxui/dom/elements.hpp"

class CategoriesWindow {
private:
    int id_;
    std::string title_;
    FilterManager* filter_manager_;

public:
    CategoriesWindow(int id, const std::string& title);

    void SetFilterManager(FilterManager* manager);
    bool HandleEvent(ftxui::Event event);
    ftxui::Element Render(bool is_selected) const;
};
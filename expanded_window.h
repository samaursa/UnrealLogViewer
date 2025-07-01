#pragma once
#include "ftxui/dom/elements.hpp"

class ExpandedWindow {
private:
    int id_;
    std::string title_;

public:
    ExpandedWindow(int id, const std::string& title);
    ftxui::Element Render(bool is_selected, int selected_log_line) const;
};
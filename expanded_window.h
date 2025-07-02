#pragma once
#include "ftxui/dom/elements.hpp"
#include "log_entry.h"

class ExpandedWindow {
private:
    int id_;
    std::string title_;

public:
    ExpandedWindow(int id, const std::string& title);
    ftxui::Element Render(bool is_selected, const LogEntry* selected_entry) const;
};

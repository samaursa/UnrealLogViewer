#pragma once
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

class LogWindow {
private:
    int id_;
    std::string title_;
    int selected_line_;
    int scroll_offset_;

public:
    LogWindow(int id, const std::string& title);

    bool HandleEvent(ftxui::Event event);
    ftxui::Element Render(bool is_selected) const;

    int GetSelectedLine() const { return selected_line_; }
    int GetScrollOffset() const { return scroll_offset_; }
};
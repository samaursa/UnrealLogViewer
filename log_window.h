#pragma once
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "log_entry.h"
#include <vector>

class LogWindow {
private:
    int id_;
    std::string title_;
    int selected_line_;
    int scroll_offset_;
    const std::vector<LogEntry>* log_entries_;

public:
    LogWindow(int id, const std::string& title);

    void SetLogEntries(const std::vector<LogEntry>* entries);
    bool HandleEvent(ftxui::Event event);
    ftxui::Element Render(bool is_selected, int available_height = 15) const;

    int GetSelectedLine() const { return selected_line_; }
    int GetScrollOffset() const { return scroll_offset_; }
    const LogEntry* GetSelectedEntry() const;
};
